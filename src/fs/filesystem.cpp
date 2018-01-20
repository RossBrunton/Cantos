#include <stdint.h>

#include "fs/filesystem.hpp"
#include "structures/utf8.hpp"
#include "main/printk.hpp"
#include "main/errno.h"
#include "test/test.hpp"
#include "main/cpu.hpp"

namespace filesystem {
    Inode::Inode(Filesystem &fs, uint64_t number, InodeType type, uint64_t size)
        : fs(fs), inode_no(number), type(type), size(size) {
        // TODO: Not thread safe
        fs.inodes ++;
    }

    Inode::~Inode() {
        fs.inodes --;
    }


    FilePathEntry::FilePathEntry(Utf8 name, shared_ptr<FilePathEntry> parent, shared_ptr<Inode> inode)
        : parent(parent), inode(inode), name(name) {}

    error_t FilePathEntry::populate(FilePathEntry& error_loc) {
        error_t err;
        shared_ptr<Inode> dir;
        FilePathEntry &lp = logical_parent();

        if(!lp.get_inode()) {
            // Parent node does not exist, ask it to populate
            err = parent->populate(error_loc);
            if(err != 0) {
                return err;
            }
        }

        if(name == ".") {
            ref = &lp;
            return EOK;
        }

        if(name == "..") {
            ref = &lp.logical_parent();
            return EOK;
        }

        if(!lp.get_inode()) {
            // Parent node does not exist, ask it to populate
            err = parent->populate(error_loc);
            if(err != 0) {
                return err;
            }
        }

        dir = lp.get_inode();
        error_loc = *this;

        if(dir->type != TYPE_DIRECTORY) {
            error_loc = *parent;
            return ENOTDIR;
        }

        for(InodeEntry &ie : dir->children) {
            if(ie.name == name) {
                err = dir->fs.read_inode(ie.inode, inode);

                if(err) {
                    inode = nullptr;
                    return err;
                }else{
                    return EOK;
                }
            }
        }

        return ENOENT;
    }

    shared_ptr<Inode> FilePathEntry::get_inode() {
        if(ref) {
            return ref->get_inode();
        }else{
            return inode;
        }
    }

    FilePathEntry &FilePathEntry::path_parent() {
        return *parent;
    }

    FilePathEntry &FilePathEntry::logical_parent() {
        if(ref) {
            return ref->logical_parent();
        }else if(!parent) {
            return *this;
        }else{
            return *parent;
        }
    }



    error_t EmptyStorage::read(addr_logical_t addr, page::Page *&page, uint32_t count) {
        page = page::alloc(flags, count);
        return EOK;
    }


    Filesystem::~Filesystem() {
        if(inodes) {
            panic("Filesystem was destroyed without releasing all inodes");
        }
    }

    shared_ptr<FilePathEntry> parse_path(const Utf8& path, shared_ptr<FilePathEntry>& base) {
        size_t pos = 0;
        size_t oldpos = 0;
        shared_ptr<FilePathEntry> parent = base;

        while((pos = path.find('/', oldpos)) != Utf8::npos) {
            Utf8 fname = path.substr(oldpos, pos - oldpos);

            if(fname.bytes()) {
                parent = make_shared<FilePathEntry>(fname, parent);
            }

            oldpos = pos + 1;
        }

        // And the rest of the string
        Utf8 fname = path.substr(oldpos, Utf8::npos);
        if(fname.bytes()) {
            parent = make_shared<FilePathEntry>(fname, parent);
        }

        return parent;
    }

    Filesystem *rootfs = nullptr;
}

namespace _tests {
using namespace filesystem;

class TestStorage : public Storage {
public:
    error_t read(addr_logical_t addr, page::Page *&page, uint32_t count) override {
        page = page::alloc(0, count);
        uint8_t *installed = (uint8_t *)page::kinstall(page, 0);

        for(uint32_t i = 0; i < PAGE_SIZE * count; i +=4) {
            installed[i + 0] = 'T';
            installed[i + 1] = 'E';
            installed[i + 2] = 'S';
            installed[i + 3] = 'T';
        }

        page::kuninstall(installed, page);
        return EOK;
    }
};

class TestFilesystem : public Filesystem {
private:
    class TestFilesystemObject : public object::Object {
    private:
        TestFilesystem &fs;

    public:
        TestFilesystemObject(uint32_t max_pages, uint8_t page_flags, uint8_t object_flags, uint32_t offset, TestFilesystem &fs) :
            Object(max_pages, page_flags, object_flags, offset), fs(fs) {};

        page::Page *do_generate(addr_logical_t addr, uint32_t count) override {
            page::Page *page;

            error_t err = fs.get_storage()->read(addr, page, count);
            if(err) {
                return nullptr;
            }
            return page;
        }
    };

public:
    TestFilesystem(shared_ptr<Storage> us) : Filesystem(us) {}

    error_t read_inode(uint64_t inode_no, shared_ptr<Inode> &inode) override {
        if(inode_no == 1) {
            inode = make_shared<Inode>(*this, 1, TYPE_DIRECTORY, 0);
            inode->children = list<InodeEntry>();
            inode->children.push_back({1, Utf8(".")});
            inode->children.push_back({1, Utf8("..")});
            inode->children.push_back({2, Utf8("a")});
            inode->children.push_back({3, Utf8("file")});

            return EOK;
        }else if(inode_no == 2) {
            inode = make_shared<Inode>(*this, 2, TYPE_DIRECTORY, 0);
            inode->children = list<InodeEntry>();
            inode->children.push_back({2, Utf8(".")});
            inode->children.push_back({1, Utf8("..")});

            return EOK;
        }else if(inode_no == 3) {
            inode = make_shared<Inode>(*this, 3, TYPE_FILE, PAGE_SIZE);

            inode->contents = make_shared<TestFilesystemObject>(1, 0, 0, 0, *this);

            return EOK;
        }else{
            return ENOENT;
        }
    }

    error_t root_inode(shared_ptr<Inode>& inode) override {
        return read_inode(1, inode);
    }
};

class FilesystemTest : public test::TestCase {
public:
    FilesystemTest() : test::TestCase("Filesystem Test") {};

    void run_test() override {
        shared_ptr<TestFilesystem> fs = make_shared<TestFilesystem>(make_shared<TestStorage>());
        error_t err;
        shared_ptr<Inode> i;
        FilePathEntry err_loc;

        test("Root inode");
        err = fs->root_inode(i);
        assert(!err);
        assert(i);
        assert(i->inode_no == 1);

        test("File paths");
        shared_ptr<FilePathEntry> root = make_shared<FilePathEntry>(Utf8(""), nullptr, i);
        assert(root->get_inode() == i);

        shared_ptr<FilePathEntry> child = parse_path(Utf8("./././."), root);
        err = child->populate(err_loc);
        assert(!err);
        assert(child->get_inode()->inode_no == 1);

        child = parse_path(Utf8("./file"), root);
        err = child->populate(err_loc);
        assert(!err);
        assert(child->get_inode()->inode_no == 3);

        test("Bad paths");
        child = parse_path(Utf8("./nothing"), root);
        err = child->populate(err_loc);
        assert(err);

        child = parse_path(Utf8("./a/file"), root);
        err = child->populate(err_loc);
        assert(err);

        child = parse_path(Utf8("./a/a/a/file"), root);
        err = child->populate(err_loc);
        assert(err);

        test("Silly paths");
        child = parse_path(Utf8("./a/././.././a/./../file"), root);
        err = child->populate(err_loc);
        assert(!err);
        assert(child->get_inode()->inode_no == 3);

        test("Reading a file");
        vm::Map *map = cpu::current_thread()->vm.get();

        map->add_object(child->get_inode()->contents, 0x2000, 0x0, 2);
        assert(*(char *)0x2000 == 'T');
        assert(*(char *)0x2001 == 'E');
        assert(*(char *)0x2002 == 'S');
        assert(*(char *)0x2003 == 'T');

        map->remove_object(child->get_inode()->contents);
    }
};

test::AddTestCase<FilesystemTest> filesystemTest;
}

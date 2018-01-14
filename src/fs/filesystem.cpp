#include <stdint.h>

#include "fs/filesystem.hpp"
#include "structures/utf8.hpp"
#include "main/printk.hpp"
#include "main/errno.h"
#include "test/test.hpp"

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
        : name(name), parent(parent), inode(inode) {}

    error_t FilePathEntry::populate(FilePathEntry& error_loc) {
        error_t err;
        shared_ptr<Inode> dir;

        if(!parent) {
            // We have no parent, we can't really get an inode, can we?
            error_loc = *this;
            return ENOPATHBASE;
        }else{
            if(!parent->inode) {
                // Parent node does not exist, ask it to populate
                err = parent->populate(error_loc);
                if(err != 0) {
                    return err;
                }
            }

            dir = parent->inode;
        }

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

class TestFilesystem : public Filesystem {
public:
    TestFilesystem(shared_ptr<UnderlyingStorage> us) : Filesystem(us) {}

    error_t read_inode(uint64_t inode_no, shared_ptr<Inode> &inode) override {
        if(inode_no == 1) {
            inode = make_shared<Inode>(*this, 1, TYPE_DIRECTORY, 0);
            inode->children = list<InodeEntry>();
            inode->children.push_back({1, Utf8(".")});
            inode->children.push_back({1, Utf8("..")});
            inode->children.push_back({1, Utf8("a")});
            inode->children.push_back({2, Utf8("file")});

            return EOK;
        }else if(inode_no == 2) {
            inode = make_shared<Inode>(*this, 2, TYPE_FILE, PAGE_SIZE);

            //readwrite::read_data(&(i->contents), 0, PAGE_SIZE, this);

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
        shared_ptr<TestFilesystem> fs = make_shared<TestFilesystem>(nullptr);
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
        assert(root->inode == i);

        shared_ptr<FilePathEntry> child = parse_path(Utf8("./././."), root);
        err = child->populate(err_loc);
        assert(!err);
        assert(child->inode->inode_no == 1);

        child = parse_path(Utf8("./file"), root);
        err = child->populate(err_loc);
        assert(!err);
        assert(child->inode->inode_no == 2);
    }
};

test::AddTestCase<FilesystemTest> filesystemTest;
}

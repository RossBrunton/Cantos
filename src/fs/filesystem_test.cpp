#include <stdint.h>

#include "fs/filesystem.hpp"
#include "structures/utf8.hpp"
#include "main/printk.hpp"
#include "main/errno.h"
#include "test/test.hpp"
#include "main/cpu.hpp"

namespace _tests {
using namespace filesystem;

class TestStorage : public Storage {
public:
    Failable<page::Page *> read(addr_logical_t addr, uint32_t count) override {
        page::Page *page = page::alloc(0, count);
        uint8_t *installed = (uint8_t *)page::kinstall(page, 0);

        for(uint32_t i = 0; i < PAGE_SIZE * count; i +=4) {
            installed[i + 0] = 'T';
            installed[i + 1] = 'E';
            installed[i + 2] = 'S';
            installed[i + 3] = 'T';
        }

        page::kuninstall(installed, page);
        return Failable<page::Page *>(EOK, page);
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
            auto result = fs.get_storage()->read(addr, count);
            if(result) {
                return result.val;
            }else{
                return nullptr;
            }
        }
    };

public:
    TestFilesystem(shared_ptr<Storage> us) : Filesystem(us) {}

    Failable<shared_ptr<Inode>> read_inode(uint64_t inode_no) override {
        shared_ptr<Inode> inode;
        if(inode_no == 1) {
            inode = make_shared<Inode>(*this, 1, InodeType::DIRECTORY, 0);
            inode->children = vector<InodeEntry>();
            inode->children.push_back({1, Utf8(".")});
            inode->children.push_back({1, Utf8("..")});
            inode->children.push_back({2, Utf8("a")});
            inode->children.push_back({3, Utf8("file")});

            return Failable<shared_ptr<Inode>>(EOK, inode);
        }else if(inode_no == 2) {
            inode = make_shared<Inode>(*this, 2, InodeType::DIRECTORY, 0);
            inode->children = vector<InodeEntry>();
            inode->children.push_back({2, Utf8(".")});
            inode->children.push_back({1, Utf8("..")});

            return Failable<shared_ptr<Inode>>(EOK, inode);
        }else if(inode_no == 3) {
            inode = make_shared<Inode>(*this, 3, InodeType::FILE, PAGE_SIZE);

            inode->contents = make_shared<TestFilesystemObject>(1, 0, 0, 0, *this);

            return Failable<shared_ptr<Inode>>(EOK, inode);
        }else{
            return Failable<shared_ptr<Inode>>(ENOENT);
        }
    }

    Failable<shared_ptr<Inode>> root_inode() override {
        return read_inode(1);
    }
};

class FilesystemTest : public test::TestCase {
public:
    FilesystemTest() : test::TestCase("Filesystem Test") {};

    void run_test() override {
        shared_ptr<TestFilesystem> fs = make_shared<TestFilesystem>(make_shared<TestStorage>());
        error_t err;
        FilePathEntry err_loc;

        test("Root inode");
        auto result = fs->root_inode();
        assert((bool)result);
        assert(result.val);
        assert(result.val->inode_no == 1);

        test("File paths");
        shared_ptr<FilePathEntry> root = make_shared<FilePathEntry>(Utf8(""), nullptr, result.val);
        assert(root->get_inode() == result.val);

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

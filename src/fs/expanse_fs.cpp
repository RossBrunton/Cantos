#include <stdint.h>

#include "fs/expanse_fs.hpp"
#include "fs/filesystem.hpp"
#include "main/cpu.hpp"
#include "main/errno.h"
#include "main/printk.hpp"
#include "structures/utf8.hpp"
#include "test/test.hpp"

namespace expanse_fs {
using namespace filesystem;
page::Page* ExpanseFs::ExpanseFsObject::do_generate(addr_logical_t addr, uint32_t count) {
    auto result = fs.get_storage()->read(addr, count);
    if (result) {
        return result.val;
    } else {
        return nullptr;
    }
}

Failable<shared_ptr<Inode>> ExpanseFs::read_inode(uint64_t inode_no) {
    shared_ptr<Inode> inode;
    if (inode_no == 1) {
        inode = make_shared<Inode>(*this, 1, InodeType::DIRECTORY, 0);
        inode->children = vector<InodeEntry>();
        inode->children.push_back({ 1, Utf8(".") });
        inode->children.push_back({ 1, Utf8("..") });
        inode->children.push_back({ 2, Utf8("contents") });

        return Failable<shared_ptr<Inode>>(EOK, inode);
    } else if (inode_no == 2) {
        inode = make_shared<Inode>(*this, 3, InodeType::FILE, PAGE_SIZE);

        inode->contents = make_shared<ExpanseFsObject>(1, 0, 0, 0, *this);

        return Failable<shared_ptr<Inode>>(EOK, inode);
    } else {
        return Failable<shared_ptr<Inode>>(ENOENT);
    }
}

Failable<shared_ptr<Inode>> ExpanseFs::root_inode() { return read_inode(1); }
}

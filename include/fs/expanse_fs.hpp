#pragma once

#include "fs/filesystem.hpp"
#include "main/common.hpp"
#include "main/errno.h"
#include "mem/object.hpp"
#include "structures/list.hpp"
#include "structures/shared_ptr.hpp"
#include "structures/utf8.hpp"

namespace expanse_fs {
using namespace filesystem;

class ExpanseFs : public Filesystem {
private:
    class ExpanseFsObject : public object::Object {
    private:
        ExpanseFs& fs;

    public:
        ExpanseFsObject(
            uint32_t max_pages, uint8_t page_flags, uint8_t object_flags, uint32_t offset, ExpanseFs& fs)
            : object::Object(max_pages, page_flags, object_flags, offset), fs(fs){};

        page::Page* do_generate(addr_logical_t addr, uint32_t count) override;
    };

public:
    ExpanseFs(shared_ptr<Storage> us) : Filesystem(us) {}
    Failable<shared_ptr<Inode>> read_inode(uint64_t inode_no) override;
    Failable<shared_ptr<Inode>> root_inode() override;
};
}

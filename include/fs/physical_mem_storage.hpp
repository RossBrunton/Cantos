#pragma once

#include "fs/filesystem.hpp"
#include "main/common.hpp"
#include "main/errno.h"
#include "mem/object.hpp"
#include "structures/list.hpp"
#include "structures/shared_ptr.hpp"
#include "structures/utf8.hpp"

namespace physical_mem_storage {
class PhysicalMemStorage : public filesystem::Storage {
private:
    addr_phys_t base;
    uint32_t flags;

public:
    PhysicalMemStorage(addr_logical_t base, uint32_t flags) : base(base), flags(flags){};
    Failable<page::Page*> read(addr_logical_t addr, uint32_t count) override;
};
}

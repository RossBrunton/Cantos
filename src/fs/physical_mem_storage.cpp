#include <stdint.h>

#include "fs/physical_mem_storage.hpp"
#include "main/cpu.hpp"
#include "main/errno.h"
#include "main/printk.hpp"
#include "structures/utf8.hpp"
#include "test/test.hpp"

namespace physical_mem_storage {
using namespace filesystem;

Failable<page::Page*> PhysicalMemStorage::read(addr_logical_t addr, uint32_t count) {
    page::Page* page = page::create(base + addr, flags, count);

    return Failable<page::Page*>(EOK, page);
}
}

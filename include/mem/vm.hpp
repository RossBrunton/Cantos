#ifndef _HPP_MEM_VM_
#define _HPP_MEM_VM_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "mem/page.hpp"

namespace object {
    class List; // Defined in object.h
}

namespace vm {
    class Map {
    public:
        page::Page *physical_dir;
        page::page_dir_t *logical_dir;
        page::logical_tables_t *logical_tables;
        uint32_t pid;
        uint32_t task_id;
        object::List *objects;

        Map(uint32_t pid, uint32_t task_id, bool kernel);
        ~Map();
        void insert(addr_logical_t addr, page::Page *page, uint8_t page_flags);
        void clear(addr_logical_t addr, uint32_t pages);
        bool resolve_fault(addr_logical_t addr);
    };

    void table_switch(addr_phys_t table);
    void table_clear();
}

#endif

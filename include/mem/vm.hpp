#ifndef _HPP_MEM_VM_
#define _HPP_MEM_VM_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "mem/page.hpp"
#include "structures/list.hpp"
#include "structures/shared_ptr.hpp"

namespace object {
    class Object;
    class ObjectInMap;
}

namespace vm {
    class Map {
    private:
        list<unique_ptr<object::ObjectInMap>> objects_in_maps;

    public:
        page::Page *physical_dir;
        page::page_dir_t *logical_dir;
        unique_ptr<page::logical_tables_t> logical_tables;
        uint32_t pid;
        uint32_t task_id;

        Map(uint32_t pid, uint32_t task_id, bool kernel);
        ~Map();
        void insert(addr_logical_t addr, page::Page *page, uint8_t page_flags, uint32_t min, uint32_t max);
        void clear(addr_logical_t addr, uint32_t pages);
        bool resolve_fault(addr_logical_t addr);

        void add_object(const shared_ptr<object::Object> object, uint32_t base, uint32_t offset, uint32_t pages);
        void remove_object(const shared_ptr<object::Object> object);
    };

    void table_switch(addr_phys_t table);
    void table_clear();
}

#endif

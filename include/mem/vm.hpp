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
        uint32_t using_cpu = 0xffffffff;

        void invlpg(addr_logical_t addr);

    public:
        page::Page *physical_dir;
        page::page_dir_t *logical_dir;
        unique_ptr<page::logical_tables_t> logical_tables;
        uint32_t pid;
        uint32_t task_id;

        Map(uint32_t pid, uint32_t task_id, bool kernel);
        ~Map();
        void insert(int64_t addr, page::Page *page, uint8_t page_flags, uint32_t min, uint32_t max);
        void clear(int64_t addr, uint32_t pages);
        bool resolve_fault(addr_logical_t addr);

        void add_object(const shared_ptr<object::Object>& object, uint32_t base, int64_t offset, uint32_t pages);
        void remove_object(const shared_ptr<object::Object>& object);
        void remove_object_at(const shared_ptr<object::Object>& object, uint32_t base);

        void enter();
        void exit();
    };
}

#endif

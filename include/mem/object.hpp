#ifndef _HPP_MEM_OBJECT_
#define _HPP_MEM_OBJECT_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "mem/vm.hpp"
#include "mem/page.hpp"

namespace object {
    const uint8_t FLAG_AUTOFREE = 0x1;

    class MapEntry {
    public:
        uint32_t base;
        vm::Map *map;
        MapEntry *next;
    };

    class PageEntry {
    public:
        uint32_t offset;
        page::Page *page;
        PageEntry *next;
    };

    class Object;
    typedef page::Page *(* object_generator_t)(addr_logical_t addr, Object *object, uint32_t count);
    typedef void (* object_deleter_t)(page::Page *page, Object *object);

    class Object {
    public:
        MapEntry *vm_maps;
        object_generator_t generator;
        object_deleter_t deleter;
        PageEntry *pages;
        uint32_t max_pages;
        uint8_t page_flags;
        uint8_t object_flags;
        uint32_t offset; // In pages

        Object(object_generator_t generator, object_deleter_t deleter, uint32_t max_pages, uint8_t page_flags,
        uint8_t object_flags, uint32_t offset);
        ~Object();

        void add_to_vm(vm::Map *map, uint32_t base);
        void remove_from_vm(vm::Map *map);

        void shift_right(uint32_t amount);

        void generate(uint32_t addr, uint32_t count);
    };

    class List {
    public:
        Object *object;
        uint32_t base;
        List *next;
    };

    page::Page *gen_empty(addr_logical_t addr, Object *object, uint32_t count);
    void del_free(page::Page *page, Object *object);
}

#endif

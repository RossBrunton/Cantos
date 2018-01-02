#ifndef _HPP_MEM_OBJECT_
#define _HPP_MEM_OBJECT_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "mem/vm.hpp"
#include "mem/page.hpp"
#include "structures/list.hpp"
#include "structures/shared_ptr.hpp"

namespace object {
    class PageEntry {
    public:
        uint32_t offset;
        page::Page *page;
        unique_ptr<PageEntry> next;
    };

    class Object;
    typedef page::Page *(* object_generator_t)(addr_logical_t addr, Object *object, uint32_t count);
    typedef void (* object_deleter_t)(page::Page *page, Object *object);

    class Object {
    private:
        list<ObjectInMap *> objects_in_maps;

    public:
        object_generator_t generator;
        object_deleter_t deleter;
        unique_ptr<PageEntry> pages = nullptr;
        uint32_t max_pages;
        uint8_t page_flags;
        uint8_t object_flags;
        void *userdata = nullptr;

        Object(object_generator_t generator, object_deleter_t deleter, uint32_t max_pages, uint8_t page_flags,
        uint8_t object_flags, uint32_t offset);
        ~Object();

        //void shift_right(uint32_t amount);
        //void shift_left(uint32_t amount);

        void generate(uint32_t addr, uint32_t count);

        void add_object_in_map(ObjectInMap *oim);
        void remove_object_in_map(ObjectInMap *oim);
    };

    page::Page *gen_empty(addr_logical_t addr, Object *object, uint32_t count);
    void del_free(page::Page *page, Object *object);

    class ObjectInMap {
    public:
        shared_ptr<Object> object;
        vm::Map *map;
        uint32_t base;
        int64_t offset;
        uint32_t pages;

        ObjectInMap(shared_ptr<Object> object, vm::Map *map, uint32_t base, int64_t offset, uint32_t pages);
        ~ObjectInMap();
    };
}

#endif

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

    class Object {
    private:
        list<ObjectInMap *> objects_in_maps;

    public:
        unique_ptr<PageEntry> pages = nullptr;
        uint32_t max_pages;
        uint8_t page_flags;
        uint8_t object_flags;
        void *userdata = nullptr;

        Object(uint32_t max_pages, uint8_t page_flags, uint8_t object_flags, uint32_t offset);
        virtual ~Object();

        //void shift_right(uint32_t amount);
        //void shift_left(uint32_t amount);

        void generate(uint32_t addr, uint32_t count);
        virtual page::Page *do_generate(addr_logical_t addr, uint32_t count) = 0;

        void add_object_in_map(ObjectInMap *oim);
        void remove_object_in_map(ObjectInMap *oim);
    };

    class EmptyObject : public Object {
    public:
        EmptyObject(uint32_t max_pages, uint8_t page_flags, uint8_t object_flags, uint32_t offset) :
            Object(max_pages, page_flags, object_flags, offset) {};
        page::Page *do_generate(addr_logical_t addr, uint32_t count) override;
    };

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

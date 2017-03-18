#include <stdint.h>
#include <stddef.h>

#include "mem/object.hpp"
#include "mem/vm.hpp"
#include "main/printk.hpp"
#include "main/panic.hpp"
#include "mem/page.hpp"
#include "mem/kmem.hpp"

namespace object {
    /**
     * @todo Handle page flags
     */

    Object::Object(object_generator_t generator, object_deleter_t deleter, uint32_t max_pages, uint8_t page_flags,
    uint8_t object_flags) {
        this->vm_maps = NULL;
        this->generator = generator;
        this->deleter = deleter;
        this->pages = NULL;
        this->max_pages = max_pages;
        this->page_flags = page_flags;
        this->object_flags = object_flags;
    }

    Object::~Object() {
        PageEntry *page_entry;
        PageEntry *next;

        // Destroy it's VM mappings
        while(this->vm_maps) {
            this->remove_from_vm(this->vm_maps->map);
        }

        // And then destroy all the pages
        for(page_entry = this->pages; page_entry; page_entry = next) {
            this->deleter(page_entry->page, this);
            next = page_entry->next;
            delete page_entry;
        }
    }

    void Object::add_to_vm(vm::Map *map, uint32_t base) {
        List *entry;
        List *next = NULL;
        List *prev = NULL;
        PageEntry *page_entry;
        MapEntry *mentry;

        for(next = map->objects; next && next->base < base; (prev = next), (next = next->next));

        // Entry in the VM's object list
        entry = new List();
        entry->base = base;
        entry->object = this;
        entry->next = next;
        if(prev) {
            prev->next = entry;
        }else{
            map->objects = entry;
        }

        // Entry in the vm map list
        mentry = new MapEntry();
        mentry->base = base;
        mentry->map = map;
        mentry->next = this->vm_maps;
        this->vm_maps = mentry;

        // And fill in the records needed on the page table
        for(page_entry = this->pages; page_entry; page_entry = page_entry->next) {
            map->insert(base + page_entry->offset, page_entry->page, this->page_flags);
        }
    }


    void Object::remove_from_vm(vm::Map *map) {
        MapEntry *mentry;
        MapEntry *mprev = NULL;
        List *oentry;
        List *oprev = NULL;
        PageEntry *page_entry;

        // Map in the object's vm list
        for(mentry = this->vm_maps; mentry && (mentry->map != map); (mprev = mentry), (mentry = mentry->next));

        if(!mentry) {
            // Not actually in the object's VM list, abort
            return;
        }

        if(mprev) {
            mprev->next = mentry->next;
        }else{
            this->vm_maps = mentry->next;
        }

        // Object in the vm's object list
        for(oentry = map->objects; oentry && (oentry->object != this); (oprev = oentry), (oentry = oentry->next));

        if(!oentry) {
            panic("Integrety error in object -> vm mapping. Object was in virtual map, but not vice versia.");
        }

        if(oprev) {
            oprev->next = oentry->next;
        }else{
            map->objects = oentry->next;
        }


        // Now erase all the page table entries
        for(page_entry = this->pages; page_entry; page_entry = page_entry->next) {
            map->clear(mentry->base + page_entry->offset, page_count(page_entry->page));
        }

        delete oentry;
        delete mentry;

        // But wait, if the flag is set, we may have to free ourselves
        if((this->object_flags & FLAG_AUTOFREE) && !this->vm_maps) {
            delete this;
        }
    }


    void Object::generate(uint32_t addr, uint32_t count) {
        PageEntry *new_entry;
        PageEntry *next = NULL;
        PageEntry *prev = NULL;
        page_t *page;
        MapEntry *map_entry;

        if((addr / PAGE_SIZE) + count > this->max_pages) {
            count = this->max_pages - (addr / PAGE_SIZE);
        }

        for(next = this->pages; next && next->offset < addr; (prev = next), (next = next->next));

        if(next && addr + (count * PAGE_SIZE) >= next->offset) {
            count = (next->offset - addr) / PAGE_SIZE;
        }

        if(!count) return;

        page = this->generator(addr, this, count);

        new_entry = new PageEntry();
        new_entry->offset = addr;
        new_entry->page = page;
        new_entry->next = next;
        if(prev) {
            prev->next = new_entry;
        }else{
            this->pages = new_entry;
        }

        // Now update the tables
        for(map_entry = this->vm_maps; map_entry; map_entry = map_entry->next) {
            map_entry->map->insert(map_entry->base + addr, page, this->page_flags);
        }
    }


    page_t *gen_empty(addr_logical_t addr, Object *object, uint32_t count) {
        (void)addr;
        (void)object;
        return page_alloc(0, count);
    }

    void del_free(page_t *page, Object *object) {
        (void)object;
        page_free(page);
    }
}

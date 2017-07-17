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
    uint8_t object_flags, uint32_t offset) {
        this->vm_maps = NULL;
        this->generator = generator;
        this->deleter = deleter;
        this->pages = NULL;
        this->max_pages = max_pages;
        this->page_flags = page_flags;
        this->object_flags = object_flags;
        this->offset = offset;
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
            map->clear(mentry->base + page_entry->offset, page_entry->page->count());
        }

        delete oentry;
        delete mentry;

        // But wait, if the flag is set, we may have to free ourselves
        if((this->object_flags & FLAG_AUTOFREE) && !this->vm_maps) {
            delete this;
        }
    }


    void Object::shift_right(uint32_t amount) {
        PageEntry *page_entry;
        MapEntry *map_entry;
        uint32_t reduction;
        uint32_t old_offset;
        uint32_t old_count;
        bool del;
        page::Page *after;
        page::Page *before;

        for(page_entry = this->pages; page_entry; page_entry = page_entry->next) {
            old_offset = page_entry->offset;
            old_count = page_entry->page->count();
            del = false;

            if(amount * PAGE_SIZE > page_entry->offset) {
                // We are cutting off the start of this page entry
                reduction = amount * PAGE_SIZE - page_entry->offset;
                page_entry->offset = 0;

                // Check if this page is entirely gone
                if(reduction > page_entry->page->count() * PAGE_SIZE) {
                    // It's gone!
                    this->pages = page_entry->next;
                    del = true;
                }else{
                    // It's been sliced!
                    after = page_entry->page->split(reduction / PAGE_SIZE);
                    before = page_entry->page;
                    page_entry->page = after;
                    page::free(before);
                }
            }else{
                // We are just moving it to the left, no rush
                page_entry->offset -= amount * PAGE_SIZE;
            }

            // Update all the VM maps
            for(map_entry = this->vm_maps; map_entry; map_entry = map_entry->next) {
                if(del) {
                    map_entry->map->clear(map_entry->base + old_offset, old_count);
                }else{
                    map_entry->map->clear(map_entry->base + old_offset + (old_count - amount) * PAGE_SIZE, amount);
                    map_entry->map->insert(map_entry->base + page_entry->offset, page_entry->page, this->page_flags);
                }
            }

            if(del) {
                // Delete the entry and free the pages if required
                page::free(page_entry->page);
                delete page_entry;
            }
        }

        this->offset += amount;
    }


    // count is in pages, Addr is an address not in pages
    void Object::generate(uint32_t addr, uint32_t count) {
        PageEntry *new_entry;
        PageEntry *next = NULL;
        PageEntry *prev = NULL;
        page::Page *page;
        MapEntry *map_entry;
        int32_t load_addr;

        uint32_t original_addr = addr;
        load_addr = addr - offset * PAGE_SIZE;

        // If we skip some bytes at the start
        if(load_addr < 0) {
            count += (load_addr / PAGE_SIZE);
            original_addr -= load_addr;
            load_addr = 0;
        }

        // If we would go greater than the maximum number of pages, cap it
        // TODO: Do I really want to do this?
        if((load_addr / PAGE_SIZE) + count > this->max_pages) {
            count = this->max_pages - (addr / PAGE_SIZE);
        }

        // Find the location to insert the page into
        for(next = this->pages; next && next->offset < load_addr; (prev = next), (next = next->next));

        // If we are bumping into the next block of pages, then reduce the amount of pages to load appropriately
        if(next && load_addr + (count * PAGE_SIZE) >= next->offset) {
            count = (next->offset - load_addr) / PAGE_SIZE;
        }

        if(!count) return;

        // Generate the pages
        page = this->generator(original_addr, this, count);

        // And create a new entry
        new_entry = new PageEntry();
        new_entry->offset = load_addr;
        new_entry->page = page;
        new_entry->next = next;
        if(prev) {
            prev->next = new_entry;
        }else{
            this->pages = new_entry;
        }

        // Now update the tables
        for(map_entry = this->vm_maps; map_entry; map_entry = map_entry->next) {
            map_entry->map->insert(map_entry->base + load_addr, page, this->page_flags);
        }
    }


    page::Page *gen_empty(addr_logical_t addr, Object *object, uint32_t count) {
        (void)addr;
        (void)object;
        return page::alloc(0, count);
    }

    void del_free(page::Page *page, Object *object) {
        (void)object;
        page::free(page);
    }
}

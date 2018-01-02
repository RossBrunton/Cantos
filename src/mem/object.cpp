#include <stdint.h>
#include <stddef.h>

#include "mem/object.hpp"
#include "mem/vm.hpp"
#include "main/printk.hpp"
#include "main/panic.hpp"
#include "mem/page.hpp"
#include "mem/kmem.hpp"
#include "main/cpu.hpp"

namespace object {
    /**
     * @todo Handle page flags
     */

    Object::Object(object_generator_t generator, object_deleter_t deleter, uint32_t max_pages, uint8_t page_flags,
    uint8_t object_flags, uint32_t offset) :
    generator(generator), deleter(deleter), max_pages(max_pages), page_flags(page_flags), object_flags(object_flags) {}

    Object::~Object() {
        PageEntry *page_entry;
        PageEntry *next;

        // And then destroy all the pages
        for(page_entry = this->pages; page_entry; page_entry = next) {
            this->deleter(page_entry->page, this);
            next = page_entry->next;
            delete page_entry;
        }
    }


    /*void Object::shift_right(uint32_t amount) {
        PageEntry *page_entry;
        PageEntry *old_page_entry;
        MapEntry *map_entry;
        uint32_t reduction;
        uint32_t old_offset;
        uint32_t old_count;
        bool del;
        page::Page *after;
        page::Page *before;
        bool no_next = false;

        for(page_entry = this->pages; page_entry; no_next ? 0 : (page_entry = page_entry->next)) {
            old_offset = page_entry->offset;
            old_count = page_entry->page->count();
            del = false;
            no_next = false;

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
                old_page_entry = page_entry;
                page_entry = page_entry->next;
                no_next = true;
                delete old_page_entry;
            }
        }

        this->offset += amount;
    }*/

    /*void Object::shift_left(uint32_t amount) {
        PageEntry *page_entry;
        PageEntry *old_page_entry;
        MapEntry *map_entry;
        uint32_t reduction;
        uint32_t old_offset;
        uint32_t old_count;
        bool del;
        page::Page *after;
        page::Page *before;
        uint32_t upper_bound = this->max_pages * PAGE_SIZE;
        bool no_next = false;
        PageEntry *prev = nullptr;

        for(page_entry = this->pages; page_entry; no_next ? 0 : (prev = page_entry) && (page_entry = page_entry->next)) {
            old_offset = page_entry->offset;
            old_count = page_entry->page->count();
            del = false;
            no_next = false;

            printk("Offset: %x, amount: %x, count: %x, upper_bound: %x\n", page_entry->offset, amount, page_entry->page->count(), upper_bound);

            if(page_entry->offset + (amount + page_entry->page->count()) * PAGE_SIZE > upper_bound) {
                // We are cutting off the end of this page entry
                reduction = page_entry->offset + (amount + page_entry->page->count()) * PAGE_SIZE - upper_bound;

                // Check if this page is entirely gone (start point is after the upper bound
                if(page_entry->offset + amount * PAGE_SIZE > upper_bound) {
                    // It's gone!
                    if(prev) {
                        prev->next = nullptr;
                    }else{
                        this->pages = nullptr;
                    }
                    del = true;
                    printk("Deleting!\n");
                }else{
                    // It's been sliced, remove pages after the end
                    after = page_entry->page->split(page_entry->page->count() - reduction / PAGE_SIZE);
                    page::free(after);
                    page_entry->offset += amount * PAGE_SIZE;
                }
            }else{
                // We are just moving it to the right, no rush
                page_entry->offset += amount * PAGE_SIZE;
            }

            // Update all the VM maps
            for(map_entry = this->vm_maps; map_entry; map_entry = map_entry->next) {
                if(del) {
                    map_entry->map->clear(map_entry->base + old_offset, old_count);
                }else{
                    map_entry->map->clear(map_entry->base + old_offset, amount);
                    map_entry->map->insert(map_entry->base + page_entry->offset, page_entry->page, this->page_flags);
                }
            }

            if(del) {
                // Delete the entry and free the pages if required
                page::free(page_entry->page);
                old_page_entry = page_entry;
                page_entry = page_entry->next;
                no_next = true;
                delete old_page_entry;
            }
        }

        this->offset -= amount;
    }*/


    // count is in pages, Addr is an address not in pages
    void Object::generate(uint32_t addr, uint32_t count) {
        PageEntry *new_entry;
        PageEntry *next = NULL;
        PageEntry *prev = NULL;
        page::Page *page;

        // If we would go greater than the maximum number of pages, cap it
        // TODO: Do I really want to do this?
        if((addr / PAGE_SIZE) + count > this->max_pages) {
            if(addr / PAGE_SIZE > this->max_pages) {
                // It is out of range, abort
                return;
            }
            count = this->max_pages - (addr / PAGE_SIZE);
        }

        // Find the location to insert the page into
        for(next = this->pages; next && next->offset < addr; (prev = next), (next = next->next));

        // If we are bumping into the next block of pages, then reduce the amount of pages to load appropriately
        if(next && addr + (count * PAGE_SIZE) >= next->offset) {
            count = (next->offset - addr) / PAGE_SIZE;
        }

        if(!count) return;

        // Generate the pages
        page = generator(addr, this, count);

        // And create a new entry
        new_entry = new PageEntry();
        new_entry->offset = addr;
        new_entry->page = page;
        new_entry->next = next;
        if(prev) {
            prev->next = new_entry;
        }else{
            pages = new_entry;
        }

        // Now update the tables
        for(ObjectInMap *oim : objects_in_maps) {
            oim->map->insert(oim->base + addr, page, page_flags, oim->base, oim->base + oim->pages * PAGE_SIZE);
        }
    }

    void Object::add_object_in_map(ObjectInMap *oim) {
        objects_in_maps.push_front(oim);

        vm::Map *map = oim->map;

        // Fill in the pages already loaded into the vm
        for(PageEntry *page_entry = pages; page_entry; page_entry = page_entry->next) {
            map->insert(oim->base + page_entry->offset, page_entry->page, this->page_flags,
                oim->base, oim->base + oim->pages * PAGE_SIZE);
        }
    }

    void Object::remove_object_in_map(ObjectInMap *oim) {
        PageEntry *page_entry;

        // Erase all the page table entries
        for(page_entry = pages; page_entry; page_entry = page_entry->next) {
            oim->map->clear(oim->base + page_entry->offset, page_entry->page->count());
        }

        objects_in_maps.remove(oim);
    }


    ObjectInMap::ObjectInMap(shared_ptr<Object> object, vm::Map *map, uint32_t base, uint32_t offset, uint32_t pages)
        : object(object), map(map), base(base), offset(offset), pages(pages) {
        object->add_object_in_map(this);
    }

    ObjectInMap::~ObjectInMap() {
        object->remove_object_in_map(this);
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


namespace _tests {
class ObjectTest : public test::TestCase {
public:
    ObjectTest() : test::TestCase("Kernel Object Test") {};

    static page::Page *gen_inc(addr_logical_t addr, object::Object *object, uint32_t count) {
        (void)addr;
        (void)object;
        page::Page * p = page::alloc(0, count);

        uint32_t *installed = (uint32_t *)page::kinstall(p, 0);

        for(uint32_t i = 0; i < count * PAGE_SIZE / 4; i ++) {
            installed[i] = addr + (i * 4);
        }

        page::kuninstall(installed, p);

        return p;
    }

    void run_test() override {
        using namespace object;

        test("Creating a simple object");
        shared_ptr<Object> obj = make_shared<Object>(gen_inc, del_free, 5, page::PAGE_TABLE_RW, 0, 0);

        test("Installing an object");
        cpu::Status& info = cpu::info();
        vm::Map *map = info.thread->vm;

        map->add_object(obj, 0x2000, 0x0, 2);
        assert(*(int *)(0x2000) == 0x0);
        assert(*(int *)(0x2004) == 0x4);
        assert(*(int *)(0x3004) == 0x1004);

        test("Removing an object");
        map->remove_object(obj);
    }
};

test::AddTestCase<ObjectTest> objectTest;
}

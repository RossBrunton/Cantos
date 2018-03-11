#include <stdint.h>
#include <stddef.h>

#include "mem/object.hpp"
#include "mem/vm.hpp"
#include "main/printk.hpp"
#include "main/panic.hpp"
#include "mem/page.hpp"
#include "mem/kmem.hpp"
#include "main/cpu.hpp"
#include "test/test.hpp"

namespace object {
    /**
     * @todo Handle page flags
     */

    Object::Object(uint32_t max_pages, uint8_t page_flags, uint8_t object_flags, uint32_t offset) :
    max_pages(max_pages), page_flags(page_flags), object_flags(object_flags) {}

    Object::~Object() {
        PageEntry *page_entry;
        PageEntry *next;

        // And then destroy all the pages
        for(page_entry = this->pages.get(); page_entry; page_entry = next) {
            page::free(page_entry->page);
            next = page_entry->next.get();
        }

        pages = nullptr;
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
        PageEntry *next = nullptr;
        PageEntry *prev = nullptr;
        page::Page *page;

        // If we would go greater than the maximum number of pages, cap it
        // TODO: Do I really want to do this?
        if((addr / PAGE_SIZE) + count > max_pages) {
            if(addr / PAGE_SIZE > max_pages) {
                // It is out of range, abort
                return;
            }
            count = max_pages - (addr / PAGE_SIZE);
        }

        // Find the location to insert the page into
        for(next = pages.get(); next && next->offset < addr; (prev = next), (next = next->next.get()));

        // If we are bumping into the next block of pages, then reduce the amount of pages to load appropriately
        if(next && addr + (count * PAGE_SIZE) >= next->offset) {
            count = (next->offset - addr) / PAGE_SIZE;
        }

        if(!count) return;

        // Generate the pages
        page = do_generate(addr, count);
        unique_ptr<PageEntry> new_entry;

        // And create a new entry
        new_entry = make_unique<PageEntry>();
        new_entry->offset = addr;
        new_entry->page = page;
        new_entry->next = next;
        if(prev) {
            prev->next.release();
            prev->next = move(new_entry);
        }else{
            pages.release();
            pages = move(new_entry);
        }

        // Now update the tables
        for(ObjectInMap *oim : objects_in_maps) {
            oim->map->insert(oim->base + addr - oim->offset, page, page_flags, oim->base, oim->base + oim->pages * PAGE_SIZE);
        }
    }

    void Object::add_object_in_map(ObjectInMap *oim) {
        objects_in_maps.push_front(oim);

        vm::Map *map = oim->map;

        // Fill in the pages already loaded into the vm
        for(PageEntry *page_entry = pages.get(); page_entry; page_entry = page_entry->next.get()) {
            map->insert(oim->base + page_entry->offset - oim->offset, page_entry->page, this->page_flags,
                oim->base, oim->base + oim->pages * PAGE_SIZE);
        }
    }

    void Object::remove_object_in_map(ObjectInMap *oim) {
        PageEntry *page_entry;

        // Erase all the page table entries
        for(page_entry = pages.get(); page_entry; page_entry = page_entry->next.get()) {
            oim->map->clear(oim->base + page_entry->offset - oim->offset, page_entry->page->count());
        }

        objects_in_maps.remove(oim);
    }


    ObjectInMap::ObjectInMap(shared_ptr<Object> object, vm::Map *map, uint32_t base, int64_t offset, uint32_t pages)
        : object(object), map(map), base(base), offset(offset), pages(pages) {
        object->add_object_in_map(this);
    }

    ObjectInMap::~ObjectInMap() {
        object->remove_object_in_map(this);
    }


    page::Page *EmptyObject::do_generate(addr_logical_t addr, uint32_t count) {
        (void)addr;
        return page::alloc(0, count);
    }
}


namespace _tests {
class ObjectTest : public test::TestCase {
public:
    ObjectTest() : test::TestCase("Kernel Object Test") {};

    class IncObject : public object::Object {
    public:
        IncObject(uint32_t max_pages, uint8_t page_flags, uint8_t object_flags, uint32_t offset) :
            Object(max_pages, page_flags, object_flags, offset) {};

        page::Page *do_generate(addr_logical_t addr, uint32_t count) {
            (void)addr;
            page::Page * p = page::alloc(0, count);

            printk("Generating %x\n", addr);

            uint32_t *installed = (uint32_t *)page::kinstall(p, 0);

            for(uint32_t i = 0; i < count * PAGE_SIZE / 4; i ++) {
                installed[i] = addr + (i * 4);
            }

            page::kuninstall(installed, p);

            return p;
        }
    };

    void run_test() override {
        using namespace object;

        test("Creating a simple object");
        shared_ptr<Object> obj = make_shared<IncObject>(5, page::PAGE_TABLE_RW, 0, 0);

        test("Installing an object");
        vm::Map *map = cpu::current_thread()->vm.get();

        map->add_object(obj, 0x2000, 0x0, 2);
        assert(*(int *)(0x2000) == 0x0);
        assert(*(int *)(0x2004) == 0x4);
        assert(*(int *)(0x3004) == 0x1004);

        test("Installing an object with offset");
        map->add_object(obj, 0x4000, 0x2000, 2);
        assert(*(int *)(0x4000) == 0x2000);
        assert(*(int *)(0x4004) == 0x2004);
        assert(*(int *)(0x5004) == 0x3004);

        test("Loading an already populated object");
        map->add_object(obj, 0x6000, 0x1000, 4);
        assert(*(int *)(0x6000) == 0x1000);
        assert(*(int *)(0x7000) == 0x2000);
        assert(*(int *)(0x8000) == 0x3000);

        test("Removing an object");
        map->remove_object(obj);
    }
};

test::AddTestCase<ObjectTest> objectTest;
}

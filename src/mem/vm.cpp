#include <stdint.h>
#include <stddef.h>

#include "mem/vm.hpp"
#include "mem/object.hpp"
#include "mem/page.hpp"
#include "mem/kmem.hpp"
#include "main/printk.hpp"
#include "main/panic.hpp"
#include "structures/mutex.hpp"
#include "main/cpu.hpp"
#include "main/asm_utils.hpp"
#include "int/lapic.hpp"

namespace vm {
    mutex::Mutex _mutex;

    Map::Map(uint32_t pid, uint32_t task_id, bool kernel) : pid(pid), task_id(task_id) {
        uint8_t kernel_flag = kernel ? page::FLAG_KERNEL : 0;
        size_t i;

        physical_dir = page::alloc(kernel_flag, 1);
        logical_dir = (page::page_dir_t *)page::kinstall(this->physical_dir, page::PAGE_TABLE_RW);
        logical_tables = make_unique<page::logical_tables_t>();

        // Load the kernel tables into it
        for(i = 0; i < PAGE_TABLE_LENGTH; i ++) {
            if(i >= PAGE_TABLE_LENGTH - KERNEL_VM_PAGE_TABLES) {
                logical_dir->entries[i] = page::page_dir->entries[i];
            }else{
                logical_dir->entries[i] = 0;
            }
        }

        // And zero the logical tables
        for(i = 0; i < (sizeof(logical_tables->tables) / sizeof(logical_tables->tables[0])); i ++) {
            logical_tables->tables[i] = NULL;
            logical_tables->pages[i] = NULL;
        }
    }


    Map::~Map() {
        page::Page *page;
        page::page_table_t *table;

        // Remove all the objects first
        objects_in_maps.clear();

        for(int i = 0; i < PAGE_TABLE_LENGTH - KERNEL_VM_PAGE_TABLES; i ++) {
            if(logical_tables->pages[i]) {
                page = logical_tables->pages[i];
                table = logical_tables->tables[i];
                page::kuninstall(table, page);
                page::free(page);
            }
        }

        page::kuninstall(logical_dir, physical_dir);
        page::free(physical_dir);
    }


    static void _new_table(addr_logical_t addr, Map *map, uint8_t page_flags) {
        page::Page *page;
        page::page_table_t *table;
        uint32_t slot = addr >> page::PAGE_DIR_SHIFT;

        if(map->logical_tables->tables[slot]) {
            return;
        }else{
            page = page::alloc(0, 1);
            table = (page::page_table_t *)page::kinstall(page, page_flags | page::PAGE_TABLE_RW);
            for(size_t i = 0; i < PAGE_TABLE_LENGTH; i ++) {
                table->entries[i] = 0;
            }

            map->logical_tables->pages[slot] = page;
            map->logical_tables->tables[slot] = table;
            map->logical_dir->entries[slot] = page->mem_base | page::PAGE_TABLE_PRESENT | page_flags;
        }
    }


    void Map::insert(int64_t addr, page::Page *page, uint8_t page_flags, uint32_t min, uint32_t max) {
        uint32_t dir_slot;
        uint32_t page_slot;
        unsigned int i = 0;

        if(addr >= max || addr < 0) {
            return;
        }

        for(i = 0; i < page->consecutive; i ++) {
            dir_slot = addr >> page::PAGE_DIR_SHIFT;
            page_slot = (addr >> page::PAGE_TABLE_SHIFT) & page::PAGE_TABLE_MASK;

            if(addr >= min) {
                if(!logical_tables->pages[dir_slot]) {
                    _new_table(addr, this, page_flags);
                }

                logical_tables->tables[dir_slot]->entries[page_slot] =
                    (page->mem_base + i * PAGE_SIZE) | page_flags | page::PAGE_TABLE_PRESENT;
                invlpg(addr);
            }

            addr += PAGE_SIZE;

            if(addr >= max) {
                return;
            }
        }

        if(page->next) {
            insert(addr, page->next, page_flags, min, max);
        }
    }


    void Map::clear(int64_t addr, uint32_t pages) {
        uint32_t dir_slot;
        uint32_t page_slot;
        unsigned int i = 0;

        if(addr < 0) return;

        for(i = 0; i < pages; i ++) {
            dir_slot = addr >> page::PAGE_DIR_SHIFT;
            page_slot = (addr >> page::PAGE_TABLE_SHIFT) & page::PAGE_TABLE_MASK;

            logical_tables->tables[dir_slot]->entries[page_slot] = 0;
            invlpg(addr);

            addr += PAGE_SIZE;
        }
    }


    bool Map::resolve_fault(addr_logical_t addr) {
        asm volatile ("sti");

        // Loop through the objects and see if any fit
        for(unique_ptr<object::ObjectInMap> &oim : objects_in_maps) {
            if(addr >= oim->base && addr < oim->base + oim->pages * PAGE_SIZE) {
                // OK!
                uint32_t excess = addr % PAGE_SIZE;
                oim->object->generate(addr - oim->base + oim->offset - excess, 1);
                return true;
            }
        }

        return false;
    }


    void Map::add_object(const shared_ptr<object::Object>& object, uint32_t base, int64_t offset, uint32_t pages) {
        unique_ptr<object::ObjectInMap> oim = make_unique<object::ObjectInMap>(object, this, base, offset, pages);

        objects_in_maps.push_front(move(oim));
    }

    void Map::remove_object(const shared_ptr<object::Object>& object) {
        for(auto i = objects_in_maps.begin(); i != objects_in_maps.end(); ) {
            if((*i)->object == object) {
                i = objects_in_maps.erase(i);
            }else{
                i ++;
            }
        }
    }

    void Map::remove_object_at(const shared_ptr<object::Object>& object, uint32_t base) {
        for(auto i = objects_in_maps.begin(); i != objects_in_maps.end(); ) {
            if((*i)->object == object && (*i)->base == base) {
                i = objects_in_maps.erase(i);
                break;
            }else{
                i ++;
            }
        }
    }

    void Map::enter() {
        _mutex.lock();
        if(using_cpu != 0xffffffff) {
            panic("Tried to set a VM which is already owned by another CPU");
        }
        using_cpu = cpu::id();
        __asm__ volatile ("mov %0, %%cr3" : : "r"(physical_dir->mem_base));
        _mutex.unlock();
    }

    void Map::exit() {
        _mutex.lock();
        using_cpu = 0xffffffff;
        __asm__ volatile ("mov %0, %%cr3" : : "r"((addr_phys_t)page::page_dir - KERNEL_VM_BASE));
        _mutex.unlock();
    }

    void Map::invlpg(addr_logical_t addr) {
        uint32_t eflags = push_cli();
        _mutex.lock();
        if(using_cpu != 0xffffffff) {
            if(using_cpu == cpu::id()) {
                __asm__ volatile ("invlpg (%0)" : : "r"(addr));
            }else{
                lapic::send_command(lapic::CMD_INVLPG, addr, using_cpu);
            }
        }
        _mutex.unlock();
        pop_flags(eflags);
    }
}

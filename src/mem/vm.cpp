#include <stdint.h>
#include <stddef.h>

#include "mem/vm.hpp"
#include "mem/object.hpp"
#include "mem/page.hpp"
#include "mem/kmem.hpp"
#include "main/printk.hpp"
#include "main/panic.hpp"

namespace vm {
    Map::Map(uint32_t pid, uint32_t task_id, bool kernel) {
        uint8_t kernel_flag = kernel ? page::FLAG_KERNEL : 0;
        size_t i;

        this->physical_dir = page::alloc(kernel_flag, 1);
        this->logical_dir = (page::page_dir_t *)page::kinstall(this->physical_dir, page::PAGE_TABLE_RW);
        this->logical_tables = (page::logical_tables_t *)kmalloc(sizeof(page::logical_tables_t), 0);

        this->pid = pid;
        this->task_id = task_id;
        this->objects = NULL;

        // Load the kernel tables into it
        for(i = 0; i < PAGE_TABLE_LENGTH; i ++) {
            if(i >= PAGE_TABLE_LENGTH - KERNEL_VM_PAGE_TABLES) {
                this->logical_dir->entries[i].table = page::page_dir->entries[i].table;
            }else{
                this->logical_dir->entries[i].table = 0;
            }
        }

        // And zero the logical tables
        for(i = 0; i < (sizeof(this->logical_tables->tables) / sizeof(this->logical_tables->tables[0])); i ++) {
            this->logical_tables->tables[i] = NULL;
            this->logical_tables->pages[i] = NULL;
        }
    }


    Map::~Map() {
        page::Page *page;
        page::page_table_t *table;

        // Remove all the objects
        while(this->objects) {
            this->objects->object->remove_from_vm(this);
        }

        page::kuninstall(this->logical_dir, this->physical_dir);
        page::free(this->physical_dir);

        for(int i = 0; i < PAGE_TABLE_LENGTH - KERNEL_VM_PAGE_TABLES; i ++) {
            if(this->logical_tables->pages[i]) {
                page = this->logical_tables->pages[i];
                table = this->logical_tables->tables[i];
                page::kuninstall(table, page);
                page::free(page);
            }
        }

        kfree(this->logical_tables);
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

            map->logical_tables->pages[slot] = page;
            map->logical_tables->tables[slot] = table;
            map->logical_dir->entries[slot].table = page->mem_base | page::PAGE_TABLE_PRESENT | page_flags;
        }
    }


    void Map::insert(addr_logical_t addr, page::Page *page, uint8_t page_flags) {
        uint32_t dir_slot;
        uint32_t page_slot;
        unsigned int i = 0;

        for(i = 0; i < page->consecutive; i ++) {
            dir_slot = addr >> page::PAGE_DIR_SHIFT;
            page_slot = (addr >> page::PAGE_TABLE_SHIFT) & page::PAGE_TABLE_MASK;

            if(!this->logical_tables->pages[dir_slot]) {
                _new_table(addr, this, page_flags);
            }

            this->logical_tables->tables[dir_slot]->entries[page_slot].block =
                (page->mem_base + i * PAGE_SIZE) | page_flags | page::PAGE_TABLE_PRESENT;

            addr += PAGE_SIZE;
        }

        if(page->next) {
            this->insert(addr, page->next, page_flags);
        }
    }


    void Map::clear(addr_logical_t addr, uint32_t pages) {
        uint32_t dir_slot;
        uint32_t page_slot;
        unsigned int i = 0;

        for(i = 0; i < pages; i ++) {
            dir_slot = addr >> page::PAGE_DIR_SHIFT;
            page_slot = (addr >> page::PAGE_TABLE_SHIFT) & page::PAGE_TABLE_MASK;

            this->logical_tables->tables[dir_slot]->entries[page_slot].block = 0;

            addr += PAGE_SIZE;
        }
    }


    bool Map::resolve_fault(addr_logical_t addr) {
        object::List *entry;

        // Loop through the objects and see if any fit
        for(entry = this->objects; entry; entry = entry->next) {
            printk("addr: %x, entry->base: %x, max_pages: %x\n", addr, entry->base, entry->object->max_pages);
            if(addr >= entry->base && addr < entry->base + entry->object->max_pages * PAGE_SIZE) {
                // OK!
                uint32_t excess = addr % PAGE_SIZE;
                entry->object->generate(addr - entry->base + (entry->object->offset * PAGE_SIZE) - excess, 1);
                return true;
            }
        }

        return false;
    }


    void table_switch(addr_phys_t table) {
        __asm__ volatile ("mov %0, %%cr3" : : "r"(table));
    }


    void table_clear() {
        __asm__ volatile ("mov %0, %%cr3" : : "r"((addr_phys_t)page::page_dir - KERNEL_VM_BASE));
    }
}

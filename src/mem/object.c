#include <stdint.h>
#include <stddef.h>

#include "mem/object.h"
#include "mem/vm.h"
#include "mem/page.h"
#include "mem/kmem.h"
#include "main/printk.h"

/**
 * @todo Handle page flags
 */

object_t *object_alloc(object_generator_t generator, uint32_t max_pages, uint8_t page_flags) {
    object_t *obj;
    
    obj = kmalloc(sizeof(object_t), 0);
    obj->vm_maps = NULL;
    obj->generator = generator;
    obj->pages = NULL;
    obj->max_pages = max_pages;
    obj->page_flags = page_flags;
    
    return obj;
}

void object_add_to_vm(object_t *object, vm_map_t *map, uint32_t base) {
    object_list_t *entry;
    object_list_t *next = NULL;
    object_list_t *prev = NULL;
    object_page_entry_t *page_entry;
    object_map_entry_t *mentry;
    
    for(next = map->objects; next && next->base < base; (prev = next), (next = next->next));
    
    entry = kmalloc(sizeof(object_list_t), 0);
    entry->base = base;
    entry->object = object;
    entry->next = next;
    if(prev) {
        prev->next = entry;
    }else{
        map->objects = entry;
    }
    
    mentry = kmalloc(sizeof(object_map_entry_t), 0);
    mentry->base = base;
    mentry->map = map;
    mentry->next = object->vm_maps;
    object->vm_maps = mentry;
    
    for(page_entry = object->pages; page_entry; page_entry = page_entry->next) {
        vm_map_insert(base + page_entry->offset, map, page_entry->page, object->page_flags);
    }
}


void object_remove_from_vm(object_t *object, vm_map_t *map);


void object_generate(object_t *object, uint32_t addr, uint32_t count) {
    object_page_entry_t *new_entry;
    object_page_entry_t *next = NULL;
    object_page_entry_t *prev = NULL;
    page_t *page;
    object_map_entry_t *map_entry;
    
    for(next = object->pages; next && next->offset < addr; (prev = next), (next = next->next));
    
    if(next && addr + (count * PAGE_SIZE) >= next->offset) {
        count = (next->offset - addr) / PAGE_SIZE;
    }
    
    if(!count) return;
    
    page = object->generator(addr, object, count);
    
    new_entry = kmalloc(sizeof(object_page_entry_t), 0);
    new_entry->offset = addr;
    new_entry->page = page;
    new_entry->next = next;
    if(prev) {
        prev->next = new_entry;
    }else{
        object->pages = new_entry;
    }
    
    // Now update the tables
    for(map_entry = object->vm_maps; map_entry; map_entry = map_entry->next) {
        vm_map_insert(map_entry->base + addr, map_entry->map, page, object->page_flags);
    }
}


page_t *object_gen_empty(addr_logical_t addr, object_t *object, uint32_t count) {
    (void)addr;
    (void)object;
    return page_alloc(0, count);
}

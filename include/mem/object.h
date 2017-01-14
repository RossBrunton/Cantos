#ifndef _H_MEM_OBJECT_
#define _H_MEM_OBJECT_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "main/multiboot.h"
#include "mem/page.h"
#include "mem/vm.h"

typedef struct object_s object_t;

typedef page_t *(* object_generator_t)(addr_logical_t addr, object_t *object, uint32_t count);
typedef void (* object_deleter_t)(page_t *page, object_t *object);

typedef struct object_map_entry_s object_map_entry_t;
struct object_map_entry_s {
    uint32_t base;
    vm_map_t *map;
    object_map_entry_t *next;
};

typedef struct object_page_entry_s object_page_entry_t;
struct object_page_entry_s {
    uint32_t offset;
    page_t *page;
    object_page_entry_t *next;
};

struct object_s {
    object_map_entry_t *vm_maps;
    object_generator_t generator;
    object_deleter_t deleter;
    object_page_entry_t *pages;
    uint32_t max_pages;
    uint8_t page_flags;
    uint8_t object_flags;
};

#ifndef _H_DEF_OBJECT_LIST_
typedef struct object_list_s object_list_t;
#define _H_DEF_OBJECT_LIST_
#endif

struct object_list_s {
    object_t *object;
    uint32_t base;
    object_list_t *next;
};

object_t *object_alloc(object_generator_t generator, object_deleter_t deleter, uint32_t max_pages, uint8_t page_flags,
    uint8_t object_flags);
void object_add_to_vm(object_t *object, vm_map_t *map, uint32_t base);
void object_remove_from_vm(object_t *object, vm_map_t *map);
void object_generate(object_t *object, uint32_t addr, uint32_t count);
page_t *object_gen_empty(addr_logical_t addr, object_t *object, uint32_t count);
void object_del_free(page_t *page, object_t *object);

#endif

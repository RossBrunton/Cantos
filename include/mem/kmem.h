#ifndef __H_MEM_KMEM__
#define __H_MEM_KMEM__

#include <stdint.h>
#include <stddef.h>

#include "mem/page.h"
#include "main/common.h"

#define KMALLOC_RESERVED (1 << 0)

typedef struct kmem_header_s {
    int size;
} kmem_header_t;

// Start of the block of free memory, not after any headers
// Points to the first free value
typedef struct kmem_free_s kmem_free_t;
struct kmem_free_s {
    size_t size;
    addr_logical_t base;
    kmem_free_t *next;
};

typedef struct kmem_map_s {
    addr_logical_t kernel_ro_start;
    addr_logical_t kernel_ro_end;
    addr_logical_t kernel_rw_start;
    addr_logical_t kernel_rw_end;
    addr_logical_t vm_start;
    addr_logical_t vm_end;
    addr_logical_t memory_start;
    addr_logical_t memory_end;
} kmem_map_t;
extern kmem_map_t kmem_map;

void kmem_init();
void *__attribute__((alloc_size(1), malloc)) kmalloc(size_t size, uint8_t flags) ;
void kfree(void *ptr);
void kmem_clear_bottom();

#endif

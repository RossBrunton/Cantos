#ifndef __H_MEM_KMEM__
#define __H_MEM_KMEM__

#include <stdint.h>
#include <stddef.h>

typedef struct kmem_header_s {
    int size;
} kmem_header_t;

// Start of the block of free memory, not after any headers
// Points to the first free value
typedef struct kmem_free_s kmem_free_t;
typedef struct kmem_free_s {
    size_t size;
    void *base;
    kmem_free_t *next;
} kmem_free_t;

typedef struct kmem_map_s {
    void *kernel_ro_start;
    void *kernel_ro_end;
    void *kernel_rw_start;
    void *kernel_rw_end;
    void *vm_start;
    void *vm_end;
    void *memory_start;
    void *memory_end;
} kmem_map_t;
extern kmem_map_t kmem_map;

void kmem_init();
void *kmalloc(size_t size);
void kfree(void *ptr);

#endif

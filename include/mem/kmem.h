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

void kmem_init();
void *kmalloc(size_t size);
void kfree(void *ptr);

#endif

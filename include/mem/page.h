#ifndef __H_MEM_PAGES__
#define __H_MEM_PAGES__

#include <stdint.h>
#include <stddef.h>

#include "main/multiboot.h"

typedef struct page_s page_t;
typedef struct page_s {
    int page_id;
    size_t mem_base_and_free;
    uint8_t flags;
    int pid;
    page_t *next;
    page_t *prev;
} page_t;

#define PAGE_FLAG_ALLOCATED 0x01
#define PAGE_FLAG_KERNEL 0x02

#define PAGE_FREE_MASK 0xfff

#define PAGE_SIZE (1024 * 4)

void page_init(multiboot_info_t *mbi);
page_t *page_alloc(int pid, uint8_t flags);
int page_free(page_t *page);

#endif

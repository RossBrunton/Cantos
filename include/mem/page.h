#ifndef __H_MEM_PAGES__
#define __H_MEM_PAGES__

#include <stdint.h>
#include <stddef.h>

#include "main/multiboot.h"

typedef struct page_s page_t;
typedef struct page_s {
    int page_id;
    void *mem_base;
    uint8_t flags;
    int pid;
    int consecutive;
    page_t *next;
} page_t;

#define PAGE_FLAG_ALLOCATED 0x01
#define PAGE_FLAG_KERNEL 0x02

#define PAGE_FREE_MASK 0xfff

#define PAGE_SIZE 0x1000
#define PAGE_TABLE_LENGTH 1024

typedef struct page_table_entry_s {
    void *block;
} page_table_entry_t;

typedef struct page_dir_entry_s {
    void *table;
} page_dir_entry_t;

#define PAGE_TABLE_FLAGMASK 0xfff
#define PAGE_TABLE_PRESENT 0x01
#define PAGE_TABLE_RW 0x02
#define PAGE_TABLE_USER 0x04
#define PAGE_TABLE_WRITETHROUGH 0x08
#define PAGE_TABLE_CACHEDISABLE 0x10
#define PAGE_TABLE_ACCESSED 0x20
#define PAGE_TABLE_DIRTY 0x40
#define PAGE_TABLE_SIZE 0x80
#define PAGE_TABLE_GLOBAL 0x100

#define KERNEL_VM_SIZE 0x40000000
#define KERNEL_VM_PAGES (KERNEL_VM_SIZE / PAGE_SIZE)
#define KERNEL_VM_PAGE_TABLES (KERNEL_VM_PAGES / PAGE_TABLE_LENGTH)

page_t *page_init(multiboot_info_t *mbi);
page_t *page_alloc(int pid, uint8_t flags, int count);
int page_free(page_t *page);
void page_used(page_t *page);

#endif

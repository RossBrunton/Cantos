#ifndef __H_MEM_PAGES__
#define __H_MEM_PAGES__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "main/multiboot.h"

typedef uintptr_t addr_phys_t;
typedef uintptr_t addr_logical_t;

typedef struct page_s page_t;
struct page_s {
    unsigned int page_id;
    addr_phys_t mem_base;
    uint8_t flags;
    unsigned int pid;
    unsigned int consecutive;
    page_t *next;
};

#define PAGE_FLAG_ALLOCATED 0x01
#define PAGE_FLAG_KERNEL 0x02
#define PAGE_FLAG_AUTOKMALLOC 0x4

#define PAGE_FREE_MASK 0xfff

#define PAGE_SIZE 0x1000
#define PAGE_DIR_SIZE 0x400000
#define PAGE_TABLE_LENGTH 0x400

#define PAGE_TABLE_SHIFT 12
#define PAGE_TABLE_MASK 0x3ff
#define PAGE_DIR_SHIFT 22

#define TOTAL_VM_SIZE 0x100000000
#define KERNEL_VM_SIZE 0x40000000
#define KERNEL_VM_PAGES (KERNEL_VM_SIZE / PAGE_SIZE)
#define KERNEL_VM_PAGE_TABLES (KERNEL_VM_PAGES / PAGE_TABLE_LENGTH)
#define KERNEL_VM_BASE (TOTAL_VM_SIZE - KERNEL_VM_SIZE) // Higher half kernel

typedef struct page_table_entry_s {
    uint32_t block;
} page_table_entry_t;

typedef struct page_table_s {
    page_table_entry_t entries[PAGE_TABLE_LENGTH];
} page_table_t;

typedef struct page_dir_entry_s {
    uint32_t table;
} page_dir_entry_t;

typedef struct page_dir_s {
    page_dir_entry_t entries[PAGE_TABLE_LENGTH];
} page_dir_t;

typedef struct page_logical_tables_s {
    page_t *pages[PAGE_TABLE_LENGTH - (KERNEL_VM_PAGE_TABLES)];
    page_table_t *tables[PAGE_TABLE_LENGTH - (KERNEL_VM_PAGE_TABLES)];
} page_logical_tables_t;

typedef struct page_vm_map_s {
    page_t *physical_dir;
    page_dir_t *logical_dir;
    page_logical_tables_t *logical_tables;
    uint32_t pid;
    uint32_t task_id;
} page_vm_map_t;

#define PAGE_TABLE_NOFLAGS(x) ((x) & ~PAGE_TABLE_FLAGMASK)
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

void page_init();
page_t *page_alloc(int pid, uint8_t flags, unsigned int count);
page_t *page_create(int pid, uint32_t base, uint8_t flags, unsigned int count);
int page_free(page_t *page);
void page_used(page_t *page);
void *page_kinstall(page_t *page, uint8_t page_flags);

page_vm_map_t *page_alloc_vm_map(uint32_t pid, uint32_t task_id, bool kernel);
bool page_vm_map_new_table
    (addr_logical_t addr, page_vm_map_t *map, page_t **page, page_table_t **table, uint8_t page_flags);
void page_vm_map_insert(addr_logical_t addr, page_vm_map_t *map, page_t *page, uint8_t page_flags);
void page_free_vm_map(page_vm_map_t *map);

void page_table_switch(addr_phys_t table);
void page_table_clear();

#endif

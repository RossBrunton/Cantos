#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "mem/page.h"
#include "mem/kmem.h"

extern char _startofro;
extern char _endofro;
extern char _endofrw;

mm_entry_t low_mb_mem_table[LOCAL_MM_COUNT];
char low_mb_cmdline[LOCAL_CMDLINE_LENGTH];
char low_mb_boot_loader_name[LOCAL_BOOT_LOADER_NAME_LENGTH];

static void *_memcpy(void *destination, const void *source, size_t num) {
    size_t i;
    for(i = 0; i < num; i ++) {
        ((char *)destination)[i] = ((char *)source)[i];
    }
    return destination;
}

static char *_strncpy(char *destination, const char *source, size_t n) {
    size_t i;
    for(i = 0; i < n; i ++) {
        destination[i] = source[i];
        if(destination[i] == '\0') {
            break;
        }
    }
    return destination;
}

volatile page_dir_t *low_kernel_main(multiboot_info_t *mbi, unsigned int magic) {
    (void)magic;
    void *low_ro_start = &_startofro - KERNEL_VM_BASE;
    void *low_ro_end = &_endofro - KERNEL_VM_BASE;
    void *low_rw_end = &_endofrw - KERNEL_VM_BASE;
    uint32_t i;
    uint32_t j;
    volatile page_dir_t *dir;
    volatile page_table_t *table;
    volatile page_table_entry_t *entry;
    kmem_map_t map_low;
    mm_entry_t *mm_entry;
    
    // Load multiboot information
    _strncpy((char *)&low_mb_cmdline, (char *)mbi->cmdline, LOCAL_CMDLINE_LENGTH);
    low_mb_cmdline[LOCAL_CMDLINE_LENGTH-1] = '\0';
    
    _strncpy((char *)&low_mb_boot_loader_name, (char *)mbi->boot_loader_name, LOCAL_BOOT_LOADER_NAME_LENGTH);
    low_mb_boot_loader_name[LOCAL_BOOT_LOADER_NAME_LENGTH-1] = '\0';
    
    mm_entry = (void*)mbi->mmap_addr;
    for(i = 0; (uint32_t)((void *)mm_entry - mbi->mmap_addr) < mbi->mmap_length && i < LOCAL_MM_COUNT; i ++) {
        _memcpy(&(low_mb_mem_table[i]), mm_entry, sizeof(mm_entry_t));
        mm_entry = (mm_entry_t *)(((void *)mm_entry) + mm_entry->size + 4);
    }
    
    // Fill in kernel map
    map_low.kernel_ro_start = low_ro_start;
    map_low.kernel_ro_end = low_ro_end;
    map_low.kernel_rw_start = low_ro_end;
    map_low.kernel_rw_end = low_rw_end;
    map_low.vm_start = low_rw_end;
    map_low.vm_end = map_low.vm_start;
    map_low.vm_end += sizeof(page_dir_entry_t) * PAGE_TABLE_LENGTH;
    map_low.vm_end += (sizeof(page_table_entry_t) * PAGE_TABLE_LENGTH) * KERNEL_VM_PAGE_TABLES;
    map_low.memory_start = map_low.vm_end;
    
    dir = map_low.vm_start;
    table = map_low.vm_start + sizeof(page_dir_t);
    
    // Set up page mapping for the first 1MB
    dir->entries[0].table = 0x0 | PAGE_TABLE_RW | PAGE_TABLE_PRESENT | PAGE_TABLE_SIZE;
    
    // And now set up the page directory and any page entries
    for(i = 1; i < PAGE_TABLE_LENGTH; i ++) {
        if(i >= (KERNEL_VM_BASE / PAGE_DIR_SIZE)) {
            dir->entries[i].table = (uint32_t)table | PAGE_TABLE_RW | PAGE_TABLE_PRESENT;
            
            entry = (page_table_entry_t *)table;
            for(j = 0; j < PAGE_TABLE_LENGTH; j ++) {
                void *addr = (void *)((i * PAGE_DIR_SIZE) + (j * PAGE_SIZE) - (uint32_t)KERNEL_VM_BASE);
                if(addr >= map_low.kernel_ro_start && addr < map_low.kernel_ro_end + PAGE_SIZE) {
                    // Kernel text
                    entry->block = (uint32_t)addr | PAGE_TABLE_PRESENT;
                }else if(addr >= map_low.kernel_rw_start && addr < map_low.vm_end + PAGE_SIZE) {
                    // Page table
                    entry->block = (uint32_t)addr | PAGE_TABLE_RW | PAGE_TABLE_PRESENT;
                }else{
                    // Absent
                    entry->block = (uint32_t)(0x0);
                }
                entry ++;
            }
            
            table ++;
        }else{
            dir->entries[i].table = 0x0 | PAGE_TABLE_RW | PAGE_TABLE_SIZE;
        }
    }
    
    return dir;
}

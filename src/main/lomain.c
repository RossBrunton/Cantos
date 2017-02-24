#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "mem/page.h"
#include "mem/kmem.h"
#include "io/acpi.h"

extern char _startofro;
extern char _endofro;
extern char _endofrw;

/** Low memory slot for the memory table.
 *
 * Just a fixed length array, it's not scattered across entire memory. Since stuff is copied in here, it won't be
 *  accidently erased like the "real" mbi memory tables would be.
 */
mm_entry_t low_mb_mem_table[LOCAL_MM_COUNT];
/** Low memory slot for the command line.
 *
 * Will always be null terminated.
 */
char low_mb_cmdline[LOCAL_CMDLINE_LENGTH];
/** Low memory slot for the bootloader name.
 *
 * Will always be null terminated
 */
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

/** Copies values into the various low memory structures, and sets up and returns a page table for the kernel.
 *
 * Only the first @ref LOCAL_MM_COUNT mbi memory map entries will be copied, the first @ref LOCAL_CMDLINE_LENGTH bytes
 *  of the command line and the first @LOCAL_BOOT_LOADER_NAME_LENGTH bytes of the boot loader name will be copied,
 *  anything left is discarded.
 *
 * The kernel memory map is filled in using the addresses of symbols defined in the linker script (`linker.ld`).
 *
 * The page directory will be set up using a reserved region located at `map.vm_start` to `map.vm_end`. This is the page
 *  directory followed by @ref KERNEL_VM_PAGE_TABLES page tables. The directory will be set up so that the page tables
 *  located after it are used to map the last @ref KERNEL_VM_SIZE pages in memory. The entire range will be marked
 *  as present in the page directory.
 *
 * Pages will be marked as follows:
 * * Readonly (.text, .rodata) sections of the kernel will be marked as present.
 * * Read/write (.data, .bss) sections will be marked as present and read/write.
 * * The page directory and page tables will be marked as present and read/write.
 * * All other pages are marked as not present.
 *
 * In addition, the first entry in the page direcotry will be marked as a present, readwrite, 4MiB page.
 *
 * @param mbi The multiboot header, from the bootloader
 * @return The kernel page table located at @ref kmem_map.vm_start
 */
volatile page_dir_t *low_kernel_main(multiboot_info_t *mbi) {
    addr_logical_t low_ro_start = (addr_logical_t)&_startofro - KERNEL_VM_BASE;
    addr_logical_t low_ro_end = (addr_logical_t)&_endofro - KERNEL_VM_BASE;
    addr_logical_t low_rw_end = (addr_logical_t)&_endofrw - KERNEL_VM_BASE;
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
    
    mm_entry = (mm_entry_t *)mbi->mmap_addr;
    for(i = 0; (addr_phys_t)((addr_phys_t)mm_entry - mbi->mmap_addr) < mbi->mmap_length && i < LOCAL_MM_COUNT; i ++) {
        _memcpy(&(low_mb_mem_table[i]), mm_entry, sizeof(mm_entry_t));
        mm_entry = (mm_entry_t *)(((addr_phys_t)mm_entry) + mm_entry->size + 4);
    }
    
    // Find ACPI tables
    low_acpi_setup();
    
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
    
    dir = (volatile page_dir_t *)map_low.vm_start;
    table = (volatile page_table_t *)(map_low.vm_start + sizeof(page_dir_t));
    
    // Set up page mapping for the first 1MB
    dir->entries[0].table = 0x0 | PAGE_TABLE_RW | PAGE_TABLE_PRESENT | PAGE_TABLE_SIZE;
    
    // And now set up the page directory and any page entries
    for(i = 1; i < PAGE_TABLE_LENGTH; i ++) {
        if(i >= (KERNEL_VM_BASE / PAGE_DIR_SIZE)) {
            dir->entries[i].table = (uint32_t)table | PAGE_TABLE_RW | PAGE_TABLE_PRESENT;
            
            entry = (page_table_entry_t *)table;
            for(j = 0; j < PAGE_TABLE_LENGTH; j ++) {
                addr_phys_t addr = (i * PAGE_DIR_SIZE) + (j * PAGE_SIZE) - KERNEL_VM_BASE;
                if(addr >= map_low.kernel_ro_start && addr < map_low.kernel_ro_end + PAGE_SIZE) {
                    // Kernel text
                    entry->block = addr | PAGE_TABLE_PRESENT;
                }else if(addr >= map_low.kernel_rw_start && addr < map_low.vm_end + PAGE_SIZE) {
                    // Page table
                    entry->block = addr | PAGE_TABLE_RW | PAGE_TABLE_PRESENT;
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

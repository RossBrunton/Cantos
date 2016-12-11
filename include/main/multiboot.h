#ifndef _H_MAIN_MULTIBOOT_
#define _H_MAIN_MULTIBOOT_

#include <stddef.h>

//Use flags to detect if a value is set
typedef struct multiboot_info_s {
    uint32_t flags;
    uint32_t mem_lower; // [0]
    uint32_t mem_upper;
    uint32_t boot_device; // [1]
    uint32_t cmdline; // [2]
    uint32_t mods_count; // [3]
    uint32_t mods_addr;
    uint32_t elf_num; // [5]
    uint32_t elf_size;
    uint32_t elf_addr;
    uint32_t elf_shndx;
    uint32_t mmap_length; // [6]
    uint32_t mmap_addr;
    uint32_t drives_length; // [7]
    uint32_t drives_addr;
    uint32_t config_table; // [8]
    uint32_t boot_loader_name; // [9]
    uint32_t apm_table; // [10]
    uint32_t vbe_control_info; // [11]
    uint32_t vbe_mode_info;
    uint32_t vbe_mode;
    uint32_t vbe_interface_seg;
    uint32_t vbe_interface_off;
    uint32_t vbe_interface_len;
} multiboot_info_t;

typedef struct mm_entry_s {
    uint32_t size;
    uint64_t base;
    uint64_t length;
    uint32_t type;
} mm_entry_t;

#define LOCAL_MM_COUNT 10
#define LOCAL_CMDLINE_LENGTH 256
#define LOCAL_BOOT_LOADER_NAME_LENGTH 16

extern mm_entry_t mb_mem_table[LOCAL_MM_COUNT];
extern char mb_cmdline[LOCAL_CMDLINE_LENGTH];
extern char mb_boot_loader_name[LOCAL_BOOT_LOADER_NAME_LENGTH];

void mb_copy_into_high();

#endif

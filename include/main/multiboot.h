#ifndef _H_MAIN_MULTIBOOT_
#define _H_MAIN_MULTIBOOT_

//Use flags to detect if a value is set
typedef struct multiboot_info_s {
    unsigned long flags;
    unsigned long mem_lower; // [0]
    unsigned long mem_upper;
    unsigned long boot_device; // [1]
    unsigned long cmdline; // [2]
    unsigned long mods_count; // [3]
    unsigned long mods_addr;
    unsigned long elf_num; // [5]
    unsigned long elf_size;
    unsigned long elf_addr;
    unsigned long elf_shndx;
    unsigned long mmap_length; // [6]
    unsigned long mmap_addr;
    unsigned long drives_length; // [7]
    unsigned long drives_addr;
    unsigned long config_table; // [8]
    unsigned long boot_loader_name; // [9]
    unsigned long apm_table; // [10]
    unsigned long vbe_control_info; // [11]
    unsigned long vbe_mode_info;
    unsigned long vbe_mode;
    unsigned long vbe_interface_seg;
    unsigned long vbe_interface_off;
    unsigned long vbe_interface_len;
} multiboot_info_t;

#endif

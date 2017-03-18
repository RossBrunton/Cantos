#ifndef _H_MAIN_MULTIBOOT_
#define _H_MAIN_MULTIBOOT_

/** @file main/multiboot.h
 *
 * This namespace contains the multiboot information structure (which is only used by @ref lomain.h), and some values
 *  which have been extracted from it.
 *
 * There is no way to actually get the multiboot header except as part of the boot process in @ref lomain.h, which
 *  exctacts values which may be needed and stores them into appropriate values.
 */

#include <stddef.h>
#include <stdint.h>

/** The multiboot information structure, as defined in the multiboot specification.
 *
 * The `syms` field, however, is instead the four fields of an ELF kernel, with the values prepended with `elf_`.
 *
 * @sa https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Boot-information-format
 */
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

/** A multiboot information memory map entry as defined in the multiboot specification.
 *
 * Note that this structure starts at the `size` field, wheras in the specification the given structure starts from
 *  the base field.
 * 
 * @sa https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Boot-information-format
 */
typedef struct mm_entry_s {
    uint32_t size;
    uint64_t base;
    uint64_t length;
    uint32_t type;
} mm_entry_t;

/** The number of memory map entries to store locally in the @ref mb_mem_table array.
 *
 * If any more exist, they will be ignored.
 */
#define LOCAL_MM_COUNT 10
/** The number of characters to reserve for the local command line.
 *
 * If the command line is longer than this, then it will be truncated.
 */
#define LOCAL_CMDLINE_LENGTH 256
/** The number of characters to reserve for the bootloader name.
 *
 * If the name is longer than this, then it will be truncated.
 */
#define LOCAL_BOOT_LOADER_NAME_LENGTH 16

/** A copy of the memory map from the multiboot information structure.
 *
 * Entries in this array are always `sizeof(@ref mm_entry_t)` bytes long, no matter what the structure itself says. The
 *  size field will not be updated to respect this fact. The first entry in these objects (that is, offset 0) will
 *  be the `size` value.
 */
extern mm_entry_t mb_mem_table[LOCAL_MM_COUNT];
/** The command line as given by the multiboot bootloader.
 *
 * It will be null terminated, even if the original is longer than @ref LOCAL_BOOT_LOADER_NAME_LENGTH or the original
 *  string is not terminated (in which case you likely have lots of garbage data).
 */
extern char mb_cmdline[LOCAL_CMDLINE_LENGTH];
/** The boot loader name as given by the multiboot bootloader.
 *
 * It will be null terminated, even if the original is longer than @ref LOCAL_BOOT_LOADER_NAME_LENGTH or the original
 *  string is not terminated (in which case you likely have lots of garbage data).
 */
extern char mb_boot_loader_name[LOCAL_BOOT_LOADER_NAME_LENGTH];

#endif

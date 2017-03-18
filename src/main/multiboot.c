#include "main/multiboot.h"

mm_entry_t mb_mem_table[LOCAL_MM_COUNT];
char mb_cmdline[LOCAL_CMDLINE_LENGTH];
char mb_boot_loader_name[LOCAL_BOOT_LOADER_NAME_LENGTH];

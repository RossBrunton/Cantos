#include <stdint.h>

#include "main/multiboot.h"
#include "main/lomain.h"

mm_entry_t mb_mem_table[LOCAL_MM_COUNT];
char mb_cmdline[LOCAL_CMDLINE_LENGTH];
char mb_boot_loader_name[LOCAL_BOOT_LOADER_NAME_LENGTH];

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

void mb_copy_into_high() {
    _memcpy(mb_mem_table, low_mb_mem_table, sizeof(mb_mem_table));
    _strncpy(mb_cmdline, low_mb_cmdline, LOCAL_CMDLINE_LENGTH);
    _strncpy(mb_boot_loader_name, low_mb_boot_loader_name, LOCAL_BOOT_LOADER_NAME_LENGTH);
}

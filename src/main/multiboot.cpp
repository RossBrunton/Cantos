#include "main/multiboot.hpp"

namespace multiboot {
    entry_t mem_table[LOCAL_MM_COUNT];
    char cmdline[LOCAL_CMDLINE_LENGTH];
    char boot_loader_name[LOCAL_BOOT_LOADER_NAME_LENGTH];
    
    info_t header;
}

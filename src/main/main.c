#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "main/stream.h"
#include "main/vga.h"
#include "main/multiboot.h"
#include "main/printk.h"
#include "mem/page.h"

#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__i386__)
#error "Wrong architecture!"
#endif

extern char _endofelf;

void kernel_main(multiboot_info_t *mbi, unsigned int magic) {
    (void) magic;
    unsigned int i;
    mm_entry_t *entry;
    
    vga_init();
    
    printk("Cantos\n", mbi->boot_loader_name);
    printk("Booted by %s [Flags: %x, Command: %s]\n", mbi->boot_loader_name, mbi->flags, mbi->cmdline);
    printk("Main function is located at %p to %p\n", kernel_main, &_endofelf);
    printk("MMap Entries:\n");
    
    entry = (void*)mbi->mmap_addr;
    for(i = 0; (uint32_t)((void *)entry - mbi->mmap_addr) < mbi->mmap_length; i ++) {
        printk("> [%p:%d] Entry %d: 0x%llx-0x%llx @ %d\n", entry, entry->size, i, entry->base,
            entry->base + entry->length, entry->type);
        entry = (mm_entry_t *)(((void *)entry) + entry->size + 4);
    }
    
    page_init(mbi);
}

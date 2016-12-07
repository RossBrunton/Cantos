#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "main/stream.h"
#include "main/vga.h"
#include "main/multiboot.h"
#include "main/printk.h"
#include "mem/page.h"
#include "mem/kmem.h"

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
    void *a;
    void *b;
    void *c;
    
    vga_init();
    
    printk("Cantos\n", mbi->boot_loader_name);
    printk("Booted by %s [Flags: %x, Command: %s]\n", mbi->boot_loader_name, mbi->flags, mbi->cmdline);
    printk("MMap Entries:\n");
    
    entry = (void*)mbi->mmap_addr;
    for(i = 0; (uint32_t)((void *)entry - mbi->mmap_addr) < mbi->mmap_length; i ++) {
        printk("> [%p:%d] Entry %d: 0x%llx-0x%llx @ %d\n", entry, entry->size, i, entry->base,
            entry->base + entry->length, entry->type);
        entry = (mm_entry_t *)(((void *)entry) + entry->size + 4);
    }
    
    kmem_init(mbi);
    
    for(int i = 1; i < 10000; i ++) {
        a = kmalloc(i);
        b = kmalloc(10);
        kfree(a);
        c = kmalloc(20);
        kfree(b);
        kfree(c);
    }
    printk("Mallocs! %p %p %p\n", a, b, c);
}

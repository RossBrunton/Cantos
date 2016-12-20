#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "main/stream.h"
#include "main/vga.h"
#include "main/multiboot.h"
#include "main/printk.h"
#include "mem/page.h"
#include "mem/kmem.h"
#include "mem/gdt.h"
#include "interrupts/idt.h"
#include "interrupts/exceptions.h"
#include "io/pic.h"
#include "io/serial.h"

#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__i386__)
#error "Wrong architecture!"
#endif

extern char _endofelf;

void kernel_main() {
    unsigned int i;
    mm_entry_t *entry;
    page_dir_t *dir;
    void *a;
    void *b;
    void *c;
    
    mb_copy_into_high();
    
    kmem_init();
    
    // Clear the first 1MiB
    dir = kmem_map.vm_start;
    dir->entries[0].table = 0x0;
    
    serial_init();
    gdt_init();
    idt_init();
    except_init();
    
    vga_init();
    printk("Cantos\n");
    printk("Booted by %s [%s]\n", mb_boot_loader_name, mb_cmdline);
    printk("Initial memory state:\n");
    printk("Kernel start: %x\n", kmem_map.kernel_ro_start);
    printk("Kernel end: %x\n", kmem_map.kernel_rw_end);
    printk("Kernel VM table start: %x\n", kmem_map.vm_start);
    printk("Kernel VM table end: %x\n", kmem_map.vm_end);
    printk("Memory start: %x\n", kmem_map.memory_start);
    printk("Memory end: %x\n", kmem_map.memory_end);
    printk("MMap Entries:\n");
    
    pic_init();
    
    entry = &(mb_mem_table[0]);
    for(i = 0; i < LOCAL_MM_COUNT && entry->size; i ++) {
        printk("> [%p:%d] Entry %d: 0x%llx-0x%llx @ %d\n", entry, entry->size, i, entry->base,
            entry->base + entry->length, entry->type);
        entry ++;
    }
    
    while(1) {};
}

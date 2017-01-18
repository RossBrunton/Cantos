#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "structures/stream.h"
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
#include "task/task.h"
#include "main/cpu.h"
#include "mem/object.h"
#include "interrupts/wrapper.h"

#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__i386__)
#error "Wrong architecture!"
#endif

extern char _endofelf;

void myfunc() {
    int i = 0;
    printk("Hello world!\n");
    while(1) {
        task_yield();
        printk("Yield success [%d]!\n", i++);
    }
    while(1){}
}

void object_test() {
    object_t *obj;
    task_thread_t *t;
    vm_map_t *vm;
    task_thread_t *thread;
    
    t = cpu_info()->thread;
    
    while(1) {
        obj = object_alloc(object_gen_empty, object_del_free, (KERNEL_VM_BASE/ PAGE_SIZE) - 1024*5, PAGE_TABLE_RW, 0);
        object_generate(obj, 0x0, (KERNEL_VM_BASE/ PAGE_SIZE) - 1);
        object_add_to_vm(obj, t->vm, 0x0);
        object_free(obj);
        vm = vm_map_alloc(0, 0, true);
        vm_map_free(vm);
        
        thread = task_thread_create(&kernel_process, (addr_logical_t)&myfunc);
        task_yield();
        task_thread_destroy(thread);
    }
    
    while(1) {}
}

void myfunc2() {
    int i = 0;
    while(1) {
        task_yield();
        printk("Yield success 2 [%d]!\n", i++);
    }
    while(1){}
}

void __attribute__((noreturn)) kernel_main() {
    unsigned int i;
    mm_entry_t *entry;
    
    task_thread_t *thread;
    
    mb_copy_into_high();
    
    kmem_init();
    
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
    ioapic_init();
    
    cpu_init();
    task_init();
    
    /*while(1) {
        page_t *page = page_alloc(0, 1);
        void *loc = page_kinstall(page, 0);
        //loc = page_kinstall(page, 0);
        page_kuninstall(loc, page);
        page_free(page);
    }*/
    
    ioapic_enable_func(IRQ_KEYBOARD, int_wrap_io_keyboard, 0);
    
    while(1){}
    
    
    
    entry = &(mb_mem_table[0]);
    for(i = 0; i < LOCAL_MM_COUNT && entry->size; i ++) {
        printk("> [%p:%d] Entry %d: 0x%llx-0x%llx @ %d\n", entry, entry->size, i, entry->base,
            entry->base + entry->length, entry->type);
        entry ++;
    }
    
    thread = task_thread_create(&kernel_process, (addr_logical_t)&object_test);
    task_enter(thread);
}

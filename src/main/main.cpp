#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "task/task.hpp"
#include "main/cpu.hpp"
#include "mem/object.hpp"
#include "main/vga.hpp"

extern "C" {
    #include "main/multiboot.h"
    #include "main/printk.h"
    #include "mem/page.h"
    #include "mem/kmem.h"
    #include "mem/gdt.h"
    #include "int/idt.h"
    #include "int/exceptions.h"
    #include "int/numbers.h"
    #include "int/pic.h"
    #include "hw/serial.h"
    #include "int/lapic.h"
    #include "int/ioapic.h"
    #include "hw/acpi.h"
    #include "hw/pit.h"
}

#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__i386__)
#error "Wrong architecture!"
#endif

extern char _endofelf;
extern "C" void _init();

void t2() {
    int i = 0;
    while(1) {
        //printk("Thread 2 [%d]!\n", i++);
    }
}

void t1() {
    int i = 0;
    while(1) {
        //printk("Thread 1 [%d]!\n", i++);
    }
}

void object_test() {
    object::Object *obj;
    task::Thread *t;
    vm::Map *vm;
    task::Thread *thread;

    t = cpu::info()->thread;

    while(1) {
        obj = new object::Object(object::gen_empty, object::del_free, (KERNEL_VM_BASE/ PAGE_SIZE) - 1024*5, PAGE_TABLE_RW, 0);
        obj->generate(0x0, (KERNEL_VM_BASE/ PAGE_SIZE) - 1);
        obj->add_to_vm(t->vm, 0x0);
        delete obj;

        vm = new vm::Map(0, 0, true);
        delete vm;

        thread = new task::Thread(&task::kernel_process, (addr_logical_t)&t1);
        task::task_yield();
        delete thread;
    }

    while(1) {}
}

extern "C" void __attribute__((noreturn)) kernel_main() {
    unsigned int i;
    mm_entry_t *entry;

    task::Thread *thread;

    mb_copy_into_high();

    kmem_init();

    _init();

    serial_init();
    gdt_init();
    gdt_setup();
    idt_init();
    idt_setup();
    except_init();

    vga::init();
    printk("Cantos\n");
    printk("Booted by %s [%s]\n", mb_boot_loader_name, mb_cmdline);
    printk("Initial memory state:\n");
    printk("Kernel start: %x\n", kmem_map.kernel_ro_start);
    printk("Kernel end: %x\n", kmem_map.kernel_rw_end);
    printk("Kernel VM table start: %x\n", kmem_map.vm_start);
    printk("Kernel VM table end: %x\n", kmem_map.vm_end);
    printk("Memory start: %x\n", kmem_map.memory_start);
    printk("Memory end: %x\n", kmem_map.memory_end);

    printk("Machine has %d cores and %d ioapics\n", acpi::acpi_proc_count, acpi::acpi_ioapic_count);

    printk("MMap Entries:\n");

    cpu::init();
    pic_init();
    lapic_init();
    lapic_setup();
    ioapic_init();
    pit_init();

    task::init();

    lapic_awaken_others();

    /*while(1) {
        page_t *page = page_alloc(0, 1);
        void *loc = page_kinstall(page, 0);
        //loc = page_kinstall(page, 0);
        page_kuninstall(loc, page);
        page_free(page);
    }*/

    ioapic_enable_func(INT_IRQ_KEYBOARD, ioapic_keyboard, 0);

    entry = &(mb_mem_table[0]);
    for(i = 0; i < LOCAL_MM_COUNT && entry->size; i ++) {
        printk("> [%p:%d] Entry %d: 0x%llx-0x%llx @ %d\n", entry, entry->size, i, entry->base,
            entry->base + entry->length, entry->type);
        entry ++;
    }

    printk("--- Before thread\n");
    thread = new task::Thread(&task::kernel_process, (addr_logical_t)&t1);
    printk("--- Thread created\n");
    new task::Thread(&task::kernel_process, (addr_logical_t)&t2);
    printk("--- Second thread created!\n");
    task_enter(thread);
}

extern "C" void __attribute__((noreturn)) ap_main() {
    cpu::info()->awoken = true;

    idt_setup();
    gdt_setup();
    lapic_setup();

    asm("sti");

    while(true) {
        asm("hlt");
        //printk("%d,", cpu::id());
    }
}

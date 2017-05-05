#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "task/task.hpp"
#include "main/cpu.hpp"
#include "mem/object.hpp"
#include "main/vga.hpp"
#include "hw/pit.hpp"
#include "hw/ps2.hpp"
#include "int/exceptions.hpp"
#include "int/pic.hpp"
#include "int/ioapic.hpp"
#include "int/idt.hpp"
#include "int/lapic.hpp"
#include "mem/gdt.hpp"
#include "main/multiboot.hpp"
#include "mem/page.hpp"
#include "mem/kmem.hpp"
#include "structures/mutex.hpp"
#include "main/printk.hpp"
#include "structures/elf.hpp"
#include "main/panic.hpp"

extern "C" {
    #include "int/numbers.h"
    #include "hw/serial.h"
    #include "hw/acpi.h"
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
        //printk("Thread 2 [%d] by %d!\n", i++, cpu::id());
        asm("hlt");
    }
}

void t1() {
    int i = 0;
    while(1) {
        //printk("Thread 1 [%d] by %d!\n", i++, cpu::id());
        asm("hlt");
    }
}

void object_test() {
    object::Object *obj;
    task::Thread *t;
    vm::Map *vm;
    task::Thread *thread;

    t = cpu::info()->thread;

    while(1) {
        obj = new object::Object(object::gen_empty, object::del_free, (KERNEL_VM_BASE/ PAGE_SIZE) - 1024*5, page::PAGE_TABLE_RW, 0);
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
    multiboot::entry_t *entry;

    task::Thread *thread;

    kmem::init();

    _init();

    serial_init();
    gdt::init();
    gdt::setup();
    idt::init();
    idt::setup();
    exceptions::init();

    vga::init();
    printk("Cantos\n");
    printk("Booted by %s [%s]\n", multiboot::boot_loader_name, multiboot::cmdline);
    printk("Initial memory state:\n");
    printk("Kernel start: %x\n", kmem::map.kernel_ro_start);
    printk("Kernel end: %x\n", kmem::map.kernel_rw_end);
    printk("Kernel symbol info start: %x\n", kmem::map.kernel_info_start);
    printk("Kernel symbol info end: %x\n", kmem::map.kernel_info_end);
    printk("Kernel VM table start: %x\n", kmem::map.vm_start);
    printk("Kernel VM table end: %x\n", kmem::map.vm_end);
    printk("Memory start: %x\n", kmem::map.memory_start);
    printk("Memory end: %x\n", kmem::map.memory_end);

    printk("Machine has %d cores and %d ioapics\n", acpi::acpi_proc_count, acpi::acpi_ioapic_count);

    cpu::init();
    pic::init();
    lapic::init();
    lapic::setup();
    ioapic::init();
    pit::init();

    task::init();

    if(multiboot::header.flags & (1 << 5)) {
        elf::load_kernel_elf(
            multiboot::header.elf_num, multiboot::header.elf_size, multiboot::header.elf_addr,
            multiboot::header.elf_shndx);
    }

    lapic::awaken_others();

    /*while(1) {
        page::Page *page = page::alloc(0, 1);
        void *loc = page::kinstall(page, 0);
        //loc = page::kinstall(page, 0);
        page_kuninstall(loc, page);
        page_free(page);
    }*/

    ps2::init();

    printk("--- Before thread\n");
    thread = new task::Thread(&task::kernel_process, (addr_logical_t)&t1);
    printk("--- Thread created\n");
    for(int i = 0; i < 10; i ++) {
        new task::Thread(&task::kernel_process, (addr_logical_t)&t2);
    }
    printk("--- Second thread created!\n");
    task::schedule(NULL);
}

extern "C" void __attribute__((noreturn)) ap_main() {
    cpu::info()->awoken = true;

    idt::setup();
    gdt::setup();
    lapic::setup();

    asm("sti");

    task::schedule(NULL);
}

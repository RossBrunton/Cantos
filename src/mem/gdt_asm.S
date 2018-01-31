.globl gdt_setup
gdt_setup:
    lgdt (gdt_descriptor);
    mov $0x8, %ax;
    mov %ax, %ds;
    mov %ax, %es;
    mov %ax, %fs;
    mov %ax, %gs;
    mov %ax, %ss;
    ret;

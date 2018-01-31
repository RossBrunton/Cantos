.section .ap-text-start, "ax"
.global low_ap_jumper
.global low_ap_jumper_end
.type low_ap_jumper, @function
.code16
.align 4
low_ap_jumper:
    ljmp $0x0, $load_gdt
load_gdt:
    # GDT
    mov $0x0, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    # Enter protected mode
    mov %cr0, %ecx
    or $0x00000001, %ecx
    mov %ecx, %cr0

    # Load the descriptor table
    lgdt ap_gdt_size

    # Jump into our protected segment
    ljmp $0x10, $enter_protected
enter_protected:
.code32
    mov $0x8, %eax;
    mov %eax, %ds
    mov %eax, %es
    mov %eax, %fs
    mov %eax, %gs
    mov %eax, %ss

    # Install the page table
    mov low_ap_page_table, %eax
    mov %eax, %cr3

    # Enable 4MiB pages
    mov %cr4, %ecx
    or $0x00000010, %ecx
    mov %ecx, %cr4

    # And flick the switch
    mov %cr0, %ecx
    or  $0x80000000, %ecx
    mov %ecx, %cr0

    # ... Right, now we need to get a stack
    mov $1, %eax
    cpuid
    shr $24, %ebx # Shift it to the right
    imul $4, %ebx # Get the offset

    # Index into the stacks array
    add $stacks, %ebx

    # And then set the stack pointer
    mov (%ebx), %eax
    add $0x1000, %eax
    mov %eax, %esp

    # And set up the GDT
    call gdt_setup

    jmp $0x10, $ap_main

.align 8


.section .ap-data-start, "wa"
.align 8
ap_gdt:
    .long 0x0 # Null entry
    .long 0x0

    .long 0x0000ffff # Data segment
    .long 0x00cf9200

    .long 0x0000ffff # 32 bit protected code segment
    .long 0x00cf9a00

.align 8
ap_gdt_size:
    .word 0x8 * 3 - 1
ap_gdt_offset:
    .long ap_gdt

.global low_ap_page_table
low_ap_page_table:
    .word 0x0

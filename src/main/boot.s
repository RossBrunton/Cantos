# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Set up VM details
.set TOTAL_VM_SIZE, 0x100000000
.set KERNEL_VM_SIZE, 0x40000000
.set KERNEL_VM_BASE, (TOTAL_VM_SIZE - KERNEL_VM_SIZE) # Higher half kernel

# Declare a multiboot header that marks the program as a kernel. These are magic
# values that are documented in the multiboot standard. The bootloader will
# search for this signature in the first 8 KiB of the kernel file, aligned at a
# 32-bit boundary. The signature is in its own section so the header can be
# forced to be within the first 8 KiB of the kernel file.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Set up stack
.section .bss
.align 16
lo_stack_bottom:
.skip 16384 # 16 KiB
lo_stack_top:

.section .hi-bss
.align 16
stack_bottom:
.skip 16384
stack_top:

# The linker script specifies _start as the entry point to the kernel and the
# bootloader will jump to this position once the kernel has been loaded. It
# doesn't make sense to return from this function as the bootloader is gone.
.section .text
.global _start
.type _start, @function
_start:
    mov $lo_stack_top, %esp
    push %ebx
    
    call low_kernel_main
    
    # Install the page table
    mov %eax, %cr3
    
    # Enable 4MiB pages
    mov %cr4, %ecx
    or $0x00000010, %ecx
    mov %ecx, %cr4
    
    # And flick the switch
    mov %cr0, %ecx
    or  $0x80000000, %ecx
    mov %ecx, %cr0
    
    mov $stack_top, %esp
    
    call kernel_main
    
    cli
halt:   hlt
    jmp halt

# Set the size of the _start symbol to the current location '.' minus its start.
# This is useful when debugging or when you implement call tracing.
.size _start, . - _start

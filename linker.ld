/* The bootloader will look at this image and start execution at the symbol designated as the entry point. */
ENTRY(_start)

_high_offset = 0xC0000000;

/* Tell where the various sections of the object files will be put in the final kernel image. */
SECTIONS {
    /* Low memory sections */

    /* Begin putting sections at 1 MiB, a conventional place for kernels to be
       loaded at by the bootloader. */
    . = 1M;

    .text BLOCK(4K) :
    {
        *(.multiboot)
        obj/main/boot.o(.text)
        obj/main/lomain.o(.text)
        obj/hw/loacpi.o(.text)
        obj/main/loerror.o(.text)
    }

    /* Read-only data. */
    .rodata BLOCK(4K) :
    {
        obj/main/lomain.o(.rodata)
        obj/main/boot.o(.rodata)
        obj/hw/loacpi.o(.rodata)
        obj/main/loerror.o(.rodata)
    }

    /* Read-write data (initialized) */
    .data BLOCK(4K) :
    {
        obj/main/lomain.o(.data)
        obj/main/boot.o(.data)
        obj/hw/loacpi.o(.data)
        obj/main/loerror.o(.data)
    }

    /* Read-write data (uninitialized) and stack */
    .bss BLOCK(4K) :
    {
        obj/main/lomain.o(COMMON)
        obj/main/boot.o(COMMON)
        obj/hw/loacpi.o(COMMON)
        obj/main/loerror.o(COMMON)
        obj/main/lomain.o(.bss)
        obj/main/boot.o(.bss)
        obj/hw/loacpi.o(.bss)
        obj/main/loerror.o(.bss)
    }

    /* HIGH MEMORY */
    . = _high_offset + .;

    _startofro = ALIGN(4K);

    .hi-text BLOCK(4K) : AT(ADDR(.hi-text) - _high_offset)
    {
        *(.text)
        *(.text*)
    }

    /** Init data */
    .hi-init : AT(ADDR(.hi-init) - _high_offset)
    {
        *(.init)
    }

    /** Finish data */
    .hi-fini : AT(ADDR(.hi-fini) - _high_offset)
    {
        *(.fini)
    }

    .eh_frame : AT(ADDR(.eh_frame) - _high_offset)
    {
        *(.eh_frame)
    }

    /* Read-only data. */
    .hi-rodata BLOCK(4K) : AT(ADDR(.hi-rodata) - _high_offset)
    {
        *(.rodata)
        *(.rodata*)
    }

    _endofro = ALIGN(4K);
    _startofrw = ALIGN(4K);

    /* Read-write data (initialized) */
    .hi-data BLOCK(4K) : AT(ADDR(.hi-data) - _high_offset)
    {
        *(.data)
    }

    /* Read-write data (uninitialized) and stack */
    .hi-bss BLOCK(4K) : AT(ADDR(.hi-bss) - _high_offset)
    {
        *(.hi-bss)
        *(COMMON)
        *(.bss)
    }

    .jcr BLOCK(4K) : AT(ADDR(.jcr) - _high_offset) {
        *(.jcr)
    }

    .ctors : AT(ADDR(.ctors) - _high_offset) {
        *(.ctors)
    }

    .dtors : AT(ADDR(.dtors) - _high_offset) {
        *(.dtors)
    }

    _startofap = .;

    . = 4K;
    /* AP start area sections (VERY low) */
    .ap-text-start : AT(_startofap - _high_offset) {
       *(.ap-text-start)
    }

    .ap-data-start : AT(_startofap - _high_offset + SIZEOF(.ap-text-start)) {
       *(.ap-data-start)
    }

    _endofap = _startofap + SIZEOF(.ap-text-start) + SIZEOF(.ap-data-start);
    . = _endofap;
    . = ALIGN(4K);
    _endofrw = .;

    .padding BLOCK(4K) : AT(ADDR(.padding) - _high_offset) {
        BYTE(0xff);
        FILL(0x00);
    }
}

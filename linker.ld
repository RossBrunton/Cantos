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
    }
    
    /* Read-only data. */
    .rodata BLOCK(4K) :
    {
        obj/main/lomain.o(.rodata)
        obj/main/boot.o(.rodata)
    }
    
    /* Read-write data (initialized) */
    .data BLOCK(4K) :
    {
        obj/main/lomain.o(.data)
        obj/main/boot.o(.data)
    }
    
    /* Read-write data (uninitialized) and stack */
    .bss BLOCK(4K) :
    {
        obj/main/lomain.o(COMMON)
        obj/main/boot.o(COMMON)
        obj/main/lomain.o(.bss)
        obj/main/boot.o(.bss)
    }
    
    /* HIGH MEMORY */
    . = _high_offset + .;
    
    _startofro = ALIGN(4K);
    
    /* First put the multiboot header, as it is required to be put very early
       early in the image or the bootloader won't recognize the file format.
       Next we'll put the .text section. */
    .hi-text BLOCK(4K) : AT(ADDR(.hi-text) - _high_offset)
    {
        *(.text)
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
    
    _endofrw = ALIGN(4K);
}

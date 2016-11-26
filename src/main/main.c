#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "main/stream.h"
#include "main/vga.h"
#include "main/multiboot.h"

#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__i386__)
#error "Wrong architecture!"
#endif

#define MSG "Hello world!"

void kernel_main(multiboot_info_t *mbi, unsigned int magic) {
    (void) magic;
    
    vga_init();
    
    uint8_t clr = vga_entry_colour(VGA_COLOUR_WHITE, VGA_COLOUR_BLACK);
    stream_writef(&vga_string_stream, 0, &clr, "Cantos\n", mbi->boot_loader_name);
    stream_writef(&vga_string_stream, 0, &clr, "Booted by %s [Flags: %x, Command: %s]",
        mbi->boot_loader_name, mbi->flags, mbi->cmdline);
}

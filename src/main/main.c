#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "main/stream.h"
#include "main/vga.h"

#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__i386__)
#error "Wrong architecture!"
#endif

#define MSG "Hello world!"

void kernel_main(void) {
    vga_init();

    uint8_t clr = vga_entry_colour(VGA_COLOUR_BLACK, VGA_COLOUR_WHITE);
    stream_write(&vga_string_stream, MSG, sizeof(MSG), 0, &clr);
}

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
    int bytes;
    stream_writef(&vga_string_stream, 0, &clr, "Running at %X%%, %s! Currently running %p!", 0xb00b5, "hello world", kernel_main);
    stream_writef(&vga_string_stream, 0, &clr, "A negative value is %d, but a positive one is %d%n", -123, 345, &bytes);
    stream_writef(&vga_string_stream, 0, &clr, "That last message was %d bytes!", bytes);
}

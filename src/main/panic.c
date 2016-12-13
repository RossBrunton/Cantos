#include <stdarg.h>

#include "main/stream.h"
#include "main/panic.h"
#include "main/vga.h"

static uint8_t clr = VGA_COLOUR_WHITE | (VGA_COLOUR_RED << 4);

void panic(char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    
    stream_writef(&vga_string_stream, 0, &clr, "\nKERNEL PANIC: ");
    vstream_writef(&vga_string_stream, 0, &clr, fmt, va);
    
    __asm__ volatile ("cli");
    while(1) {
        __asm__ volatile ("hlt");
    }
    
    va_end(va);
}

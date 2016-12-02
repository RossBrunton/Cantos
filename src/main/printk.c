#include <stdint.h>
#include <stdarg.h>

#include "main/stream.h"
#include "main/vga.h"
#include "main/printk.h"

static uint8_t clr = VGA_COLOUR_WHITE | (VGA_COLOUR_BLACK << 4);

void printk(char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vprintk(fmt, va);
    va_end(va);
}

void vprintk(char *fmt, va_list ap) {
    vstream_writef(&vga_string_stream, 0, &clr, fmt, ap);
}

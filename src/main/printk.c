#include <stdint.h>
#include <stdarg.h>

#include "structures/stream.h"
#include "main/vga.h"
#include "main/printk.h"
#include "io/serial.h"

static uint8_t clr = VGA_COLOUR_WHITE | (VGA_COLOUR_BLACK << 4);
static uint8_t clr_warn = VGA_COLOUR_MAGENTA | (VGA_COLOUR_BLACK << 4);
static uint8_t clr_err = VGA_COLOUR_RED | (VGA_COLOUR_BLACK << 4);

void __attribute__((format(printf, 1, 2))) printk(char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vprintk(fmt, va);
    va_end(va);
}

void __attribute__((format(printf, 1, 0))) vprintk(char *fmt, va_list ap) {
    vstream_writef(&vga_string_stream, 0, &clr, fmt, ap);
    vstream_writef(&serial_all_ports, 0, NULL, fmt, ap);
}

void __attribute__((format(printf, 1, 2))) kwarn(char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vkwarn(fmt, va);
    va_end(va);
}

void __attribute__((format(printf, 1, 0))) vkwarn(char *fmt, va_list ap) {
    vstream_writef(&vga_string_stream, 0, &clr_warn, fmt, ap);
    vstream_writef(&serial_all_ports, 0, NULL, fmt, ap);
}

void __attribute__((format(printf, 1, 2))) kerror(char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vkerror(fmt, va);
    va_end(va);
}

void __attribute__((format(printf, 1, 0))) vkerror(char *fmt, va_list ap) {
    vstream_writef(&vga_string_stream, 0, &clr_err, fmt, ap);
    vstream_writef(&serial_all_ports, 0, NULL, fmt, ap);
}

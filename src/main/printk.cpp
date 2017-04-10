#include <stdint.h>
#include <stdarg.h>

#include "structures/stream.hpp"
#include "main/vga.hpp"
#include "main/printk.hpp"
#include "hw/serial.hpp"
#include "structures/mutex.hpp"

static uint8_t clr = vga::COLOUR_WHITE | (vga::COLOUR_BLACK << 4);
static uint8_t clr_warn = vga::COLOUR_MAGENTA | (vga::COLOUR_BLACK << 4);
static uint8_t clr_err = vga::COLOUR_RED | (vga::COLOUR_BLACK << 4);

static mutex::Mutex _mutex;

extern "C" {
    void __attribute__((format(printf, 1, 2))) printk(const char *fmt, ...) {
        va_list va;
        va_start(va, fmt);
        vprintk(fmt, va);
        va_end(va);
    }

    void __attribute__((format(printf, 1, 0))) vprintk(const char *fmt, va_list ap) {
        _mutex.lock();
        vga::string_stream.writef(0, &clr, fmt, ap);
        serial::all_serial_ports.writef(0, NULL, fmt, ap);
        _mutex.unlock();
    }

    void __attribute__((format(printf, 1, 2))) kwarn(const char *fmt, ...) {
        va_list va;
        va_start(va, fmt);
        vkwarn(fmt, va);
        va_end(va);
    }

    void __attribute__((format(printf, 1, 0))) vkwarn(const char *fmt, va_list ap) {
        _mutex.lock();
        vga::string_stream.writef(0, &clr_warn, fmt, ap);
        serial::all_serial_ports.writef(0, NULL, fmt, ap);
        _mutex.unlock();
    }

    void __attribute__((format(printf, 1, 2))) kerror(const char *fmt, ...) {
        va_list va;
        va_start(va, fmt);
        vkerror(fmt, va);
        va_end(va);
    }

    void __attribute__((format(printf, 1, 0))) vkerror(const char *fmt, va_list ap) {
        _mutex.lock();
        vga::string_stream.writef(0, &clr_err, fmt, ap);
        serial::all_serial_ports.writef(0, NULL, fmt, ap);
        _mutex.unlock();
    }
}

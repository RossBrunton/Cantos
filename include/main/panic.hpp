#ifndef _HPP_MAIN_PANIC_
#define _HPP_MAIN_PANIC_

#include <stdarg.h>
#include <stdint.h>

/** @file main/panic.h
 *
 * This defines the panic function, which should be called to stop the kernel when something goes seriously wrong.
 */

/** Prints a formatted message to a number of sources, and then halts the system.
 *
 * This should be called if an irrecoverable error has been encountered.
 *
 * This function is like `printf`, it takes a format parameter and some values to use in that format. Specifically, the
 *  format for @ref stream_writef.
 *
 * The error message will be sent to the VGA string stream (@ref vga_string_stream), in white text on a red background,
 *  prepended by "KERNEL PANIC: ".
 *
 * This will then disable interrupts and halt the system, meaning this function does not return.
 * 
 * @param[in] fmt The format string
 * @param[in] ... The values to use with the format
 */
extern "C" void __attribute__((format(printf, 1, 2))) panic(const char *fmt, ...);

extern "C" void vpanic_at(uint32_t ebp, uint32_t eip, const char *fmt, va_list ap);
extern "C" void __attribute__((format(printf, 3, 4))) panic_at(uint32_t ebp, uint32_t eip, const char *fmt, ...);

#endif

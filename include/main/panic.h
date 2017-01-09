#ifndef _H_MAIN_PANIC_
#define _H_MAIN_PANIC_

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
void panic(char *fmt, ...);

#endif

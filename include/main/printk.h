#ifndef __H_MAIN_PRINTK__
#define __H_MAIN_PRINTK__

/** @file main/printk.h
 *
 * Contains a number of functions to write text to a "log".
 *
 * Specifically, these functions write to the @ref vga_string_stream and all of the serial ports.
 *
 * There are three "log levels":
 * * "normal", done using the "print" function which is for normal logging information.
 * * "warning", for conditions that will likely cause problems, but aren't a failure. This is coloured yellow where
 *  possible.
 * * "error", for conditions that indicate an error.
 *
 * If you want to halt the kernel's execution, @ref panic should be called instead, these functions are purely for
 *  relaying information.
 *
 * All of these functions are similar to printf, in that they take a format string and a number of arguments.
 *  Specifically, the format must be usable by @ref stream_writef.
 */

#include <stdarg.h>

/** Prints a regular message to the log destination.
 *
 * @param[in] fmt The format of the message
 * @param[in] ... Parameters for the format
 */
void printk(char *fmt, ...);
/** Prints a regular message to the log destination.
 *
 * @param[in] fmt The format of the message
 * @param[in] ap The argument list from `va_start`
 */
void vprintk(char *fmt, va_list ap);

/** Prints a warning message to the log destination.
 *
 * @param[in] fmt The format of the message
 * @param[in] ... Parameters for the format
 */
void kwarn(char *fmt, ...);
/** Prints a warning message to the log destination.
 *
 * @param[in] fmt The format of the message
 * @param[in] ap The argument list from `va_start`
 */
void vkwarn(char *fmt, va_list ap);

/** Prints an error message to the log destination.
 *
 * @param[in] fmt The format of the message
 * @param[in] ... Parameters for the format
 */
void kerror(char *fmt, ...);
/** Prints an error message to the log destination.
 *
 * @param[in] fmt The format of the message
 * @param[in] ap The argument list from `va_start`
 */
void vkerror(char *fmt, va_list ap);

#endif

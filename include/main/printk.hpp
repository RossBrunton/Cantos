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

#ifdef __cplusplus
extern "C" {
#endif
    /** Prints a regular message to the log destination.
     *
     * @param[in] fmt The format of the message
     * @param[in] ... Parameters for the format
     */
    void __attribute__((format(printf, 1, 2))) printk(const char *fmt, ...);
    /** Prints a regular message to the log destination.
     *
     * @param[in] fmt The format of the message
     * @param[in] ap The argument list from `va_start`
     */
    void __attribute__((format(printf, 1, 0))) vprintk(const char *fmt, va_list ap);

    /** Prints a warning message to the log destination.
     *
     * @param[in] fmt The format of the message
     * @param[in] ... Parameters for the format
     */
    void __attribute__((format(printf, 1, 2))) kwarn(const char *fmt, ...);
    /** Prints a warning message to the log destination.
     *
     * @param[in] fmt The format of the message
     * @param[in] ap The argument list from `va_start`
     */
    void __attribute__((format(printf, 1, 0))) vkwarn(const char *fmt, va_list ap);

    /** Prints an error message to the log destination.
     *
     * @param[in] fmt The format of the message
     * @param[in] ... Parameters for the format
     */
    void __attribute__((format(printf, 1, 2))) kerror(const char *fmt, ...);
    /** Prints an error message to the log destination.
     *
     * @param[in] fmt The format of the message
     * @param[in] ap The argument list from `va_start`
     */
    void __attribute__((format(printf, 1, 0))) vkerror(const char *fmt, va_list ap);
#ifdef __cplusplus
}
#endif

#endif

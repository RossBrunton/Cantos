#ifndef _H_MAIN_ERRNO_
#define _H_MAIN_ERRNO_

/** @file main/errno.h
 *
 * Contains the definition of `errno` and some of its values.
 *
 * `errno` may be set to one of these values by some functions in the kernel, as per "normal" C library functions.
 *
 * The error codes are from POSIX, although at the current time not all of them are implemented.
 */

/** The current error code */
extern volatile int errno;

/** The type used to represent errors */
typedef uint32_t error_t;

/** Operation not permitted. */
#define EOK 0
#define EPERM 1
#define EBUSY 2

#endif

#ifndef _H_MAIN_ERRNO_
#define _H_MAIN_ERRNO_

/** @file main/errno.h
 *
 * Contains the definition of `errno` and some of its values.
 *
 * `errno` may be set to one of these values by some functions in the kernel, as per "normal" C library functions.
 *
 * Some error codes are from posix, but some are not.
 */

/** Operation not permitted. */
enum error_t {
    EOK = 0,
    EPERM,
    EBUSY,
    ENOENT,
    ENOTDIR,
    ENOPATHBASE /* No base for path */
};

#endif

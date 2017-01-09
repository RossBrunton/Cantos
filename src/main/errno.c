/** @file main/errno.c
 *
 * Defines the errno variable. At the moment its just a regular int.
 * 
 * @todo This will cause race conditions in the kernel if an interrupt causes errno to be written.
 * @todo Make it processor local
 */

/** The current error code */
volatile int errno;

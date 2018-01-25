/** @file
 *
 * Global configuration constants, turn on or off these flags to configure the kernel
 */

/** @name Testing
 *
 * @{
 */
/** If defined, tests will be enabled
 *
 * When undefined, the list of tests will not be populated (and subject to removal by `-O2`).
 *
 * @see test
 */
#define TESTS 1
/** @} */

/** @name Additional Logging
 *
 * @{
 */
/** If defined, then the kernel memory map will be printed on boot */
#define DEBUG_MAP 0

/** If defined, then extra debug information will be printk'd by the memory subsystem */
#define DEBUG_MEM 0

/** If defined, then a LOT of extra debug information will be printk'd by the memory subsystem */
#define DEBUG_VMEM 0

/** If defined, then extra debug information will be printk'd by the PCI driver */
#define DEBUG_PCI 0

/** If defined, then extra debug information will be printk'd by the PS2 driver */
#define DEBUG_PS2 0

/** If defined, then extra debug information will be printk'd by the serial port driver */
#define DEBUG_SERIAL 0
/** @} */

/** @name Memory Management
 *
 * @{
 */
/** If defined, then the memory system will write a sentinel value to the memory header structure
 *
 * This MAY catch some memory allocation bugs, however a lot more memory will be used up.
 **/
#define KMEM_SENTINEL 1

/** @name Code Checks
 *
 * @{
 */
/** If defined, checks will be made in certain areas of the code as to whether interrupts or enabled or not
 *
 * This should help pick up areas where interrupts should be enabled/disabled in certain areas, but are not.
 */
#define CHECK_IF 0
/** @} */

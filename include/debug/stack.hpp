#pragma once

#include <stdint.h>

#include "main/common.hpp"
#include "structures/elf.hpp"

/** Contains tools for working with a stack */
namespace stack {
/** An Unwinder takes an ebp register value of an existing stack, and then "unwinds" the stack, giving the method name
 *  at each step.
 *
 * This only works if source files are compiled with -fno-omit-frame-pointer, and likely won't work at high optimisation
 *  levels.
 *
 * The Unwinder is focused on a specific stack frame at a given time. @ref unwind can be used to climb up the stack.
 */
class Unwinder {
public:
    /** The ebp value of the current stack frame */
    addr_logical_t ebp;

    /** Create a new stack frame
     *
     * @param ebp The value of the ebp register in the code being inspected
     */
    Unwinder(addr_logical_t ebp);

    /** Climbs up to the previous stack frame
     *
     * If there are no more stack frames, then this will return false, and the focused method will be undefined.
     *
     * @return True if the new stack frame is valid
     */
    bool unwind();
    /** Get the return address of the current stack frame
     *
     * @return The logical address of the return address
     */
    addr_logical_t getReturn() const;
    /** Get the method name of the function calling the current stack frame
     *
     * Note that is the caller of that method, not the currently inspected method itself.
     *
     * @param elf The elf headers of the code that this stack refers to
     * @return The name of the method
     */
    const char* methodName(const elf::Header& elf) const;
};
}

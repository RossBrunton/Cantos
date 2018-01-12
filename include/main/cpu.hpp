#ifndef _HPP_MAIN_CPU_
#define _HPP_MAIN_CPU_

/** Provides various mechanisms to working with CPU state
 *
 * This currently consists of managing a `cpu_status_t` structure, which gives information on a given processor.
 *
 * Please be aware of race conditions and concurrency issues while using this. The processor a process is running on may
 *  change at any time.
 */

#include <stdint.h>

#include "task/task.hpp"
#include "hw/acpi.h"
#include "structures/shared_ptr.hpp"

namespace cpu {
    /** Represents the current state of a CPU
     *
     * This structure is bound to a specific CPU, call `cpu_info` to get the structure for the currently running CPU
     */
    class Status {
    public:
        uint8_t cpu_id; /**< The ID of the CPU, each CPU has a unique value */
        void *stack; /**< A pointer to the start of the CPU's kernel stack */
        shared_ptr<task::Thread> thread; /**< The thread this CPU is currently running, or NULL if it is not running one */
        bool awoken; /**< Whether the CPU has been woken up yet */
    };

    extern "C" addr_logical_t *stacks[MAX_CORES];

    const uint32_t CF = 0x1;
    const uint32_t PF = 0x4;
    const uint32_t AF = 0x8;
    const uint32_t ZF = 0x20;
    const uint32_t SF = 0x40;
    const uint32_t TF = 0x80;
    const uint32_t IF = 0x100;
    const uint32_t DF = 0x200;
    const uint32_t OF = 0x400;

    /** Returns the CPU status structure for currently executing CPU
     *
     * @return The CPU's status structure
     */
    Status& info();
    /** Returns the CPU status structure for another CPU.
     *
     * @param [in] id The CPU id to get status for.
     * @return That CPU's status structure.
     */
    Status& info_of(uint32_t id);
    /** Initializes the "cpu" namespace, call this before using any functions in this file.
     *
     * This also creates a stack for the CPU to use, so it should be called before doing a task switch. It also requires
     *  memory allocation to be working
     */
    void init();
    /** Returns the currently running thread
     *
     * Reads the "thread" property of the CPU, but disables interrupts to avoid race conditions
     */
    shared_ptr<task::Thread> current_thread();
    /** Returns the currently running thread without disabling interrupts
     */
    shared_ptr<task::Thread> current_thread_noint();

    uint32_t id();
}

#endif

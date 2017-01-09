#ifndef _H_MAIN_CPU_
#define _H_MAIN_CPU_

/** @file main/cpu.h 
 *
 * Provides various mechanisms to working with CPU state.
 *
 * This currently consists of managing a `cpu_status_t` structure, which gives information on a given processor.
 */

#include <stdint.h>

#include "task/task.h"

/** Represents the current state of a CPU.
 *
 * This structure is bound to a specific CPU, call `cpu_info` to get the structure for the currently running CPU.
 */
typedef struct task_cpu_status_s {
    uint8_t cpu_id; /**< The ID of the CPU, each CPU has a unique value */
    void *stack; /**< A pointer to the start of the CPU's kernel stack */
    task_thread_t *thread; /**< The thread this CPU is currently running, or NULL if it is not running one */
} cpu_status_t;

/** Returns the CPU status structure for currently executing CPU.
 *
 * @return The CPU's status structure.
 */
cpu_status_t *cpu_info();
/** Initializes the "cpu" namespace, call this before using any functions in this file.
 *
 * This also creates a stack for the CPU to use, so it should be called before doing a task switch. It also requires
 *  memory allocation to be working.
 */
void cpu_init();

#endif

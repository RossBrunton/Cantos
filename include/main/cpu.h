#ifndef _H_MAIN_CPU_
#define _H_MAIN_CPU_

#include <stdint.h>

#include "task/task.h"

typedef struct task_cpu_status_s {
    uint8_t cpu_id;
    void *stack;
    task_thread_t *thread;
} cpu_status_t;

cpu_status_t *cpu_info();
void cpu_init();

#endif

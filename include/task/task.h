#ifndef _H_TASK_TASK_
#define _H_TASK_TASK_

#include <stddef.h>

#include "main/common.h"

void __attribute__((noreturn)) task_enter(void *thread);
void task_yield();
void task_yield_done(uint32_t sp);
void task_timer_yield();

#endif

#ifndef _H_TASK_ASM_
#define _H_TASK_ASM_

#include <stdint.h>

void task_asm_entry_point();
void __attribute__((fastcall)) task_asm_enter(uint32_t sp);

#endif

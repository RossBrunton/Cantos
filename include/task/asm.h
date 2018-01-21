#ifndef _H_TASK_ASM_
#define _H_TASK_ASM_

#include <stdint.h>

void task_asm_entry_point();
void __attribute__((fastcall,noreturn)) task_asm_enter(uint32_t sp);
int __attribute__((fastcall)) task_asm_yield(uint32_t sp);
int __attribute__((fastcall)) task_asm_set_stack(uint32_t sp, void (*next)());

#endif

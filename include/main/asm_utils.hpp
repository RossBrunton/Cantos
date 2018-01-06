#ifndef _H_ASM_UTILS_
#define _H_ASM_UTILS_

#include <stdint.h>

extern "C" {
uint32_t push_flags();
uint32_t push_cli();
void __attribute__((fastcall)) pop_flags(uint32_t flags);
}

#endif

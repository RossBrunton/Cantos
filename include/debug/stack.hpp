#ifndef _HPP_MAIN_STACK_
#define _HPP_MAIN_STACK_

#include <stdint.h>

#include "main/common.hpp"
#include "structures/elf.hpp"

namespace stack {
    class Unwinder {
    public:
        addr_logical_t ebp;
        
        Unwinder(addr_logical_t ebp);
        
        bool unwind();
        addr_logical_t getReturn();
        char *methodName(elf::Header *elf);
    };
}

#endif

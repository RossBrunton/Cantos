#include <stdint.h>

#include "structures/elf.hpp"
#include "debug/stack.hpp"

namespace stack {
    Unwinder::Unwinder(addr_logical_t ebp) {
        this->ebp = ebp;
    }
    
    addr_logical_t Unwinder::getReturn() {
        uint32_t *addr = (uint32_t *)this->ebp;
        
        return *(addr + 1);
    }
    
    char *Unwinder::methodName(elf::Header *elf) {
        addr_logical_t ret = this->getReturn();
        
        return elf->runtimeFindSymbolName(ret, elf::st_info(elf::STB_GLOBAL, elf::STT_FUNC));
    }
    
    bool Unwinder::unwind() {
        this->ebp = *((uint32_t *)this->ebp + 0);
        
        return this->ebp > 0;
    }
}

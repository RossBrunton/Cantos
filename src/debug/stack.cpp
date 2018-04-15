#include <stdint.h>

#include "debug/stack.hpp"
#include "structures/elf.hpp"

namespace stack {
Unwinder::Unwinder(addr_logical_t ebp) { this->ebp = ebp; }

addr_logical_t Unwinder::getReturn() const {
    const uint32_t* addr = (uint32_t*)this->ebp;

    return *(addr + 1);
}

const char* Unwinder::methodName(const elf::Header& elf) const {
    const addr_logical_t ret = this->getReturn();

    return elf.runtimeFindSymbolName(ret, elf::st_info(elf::STB_GLOBAL, elf::STT_FUNC));
}

bool Unwinder::unwind() {
    this->ebp = *((uint32_t*)this->ebp + 0);

    return this->ebp > 0;
}
}

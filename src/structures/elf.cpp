#include <stdint.h>

#include "structures/elf.hpp"
#include "mem/kmem.hpp"
#include "main/printk.hpp"
#include "main/common.h"

namespace elf {
    static void *_memcpy(void *destination, const void *source, size_t num) {
        size_t i;
        for(i = 0; i < num; i ++) {
            ((char *)destination)[i] = ((char *)source)[i];
        }
        return destination;
    }

    SectionHeader *Header::sectionHeader(uint32_t id) {
        return (SectionHeader *)((addr_logical_t)this + this->shoff + (id * this->shentsize));
    }

    SectionHeader *Header::sectionStringSection() {
        return this->sectionHeader(this->shstrndx);
    }

    void *Header::sectionData(uint32_t id) {
        SectionHeader *hdr = this->sectionHeader(id);

        return (void *)((addr_logical_t)this + hdr->offset);
    }

    void *Header::runtimeSectionData(uint32_t id) {
        SectionHeader *hdr = this->sectionHeader(id);

        return (void *)hdr->addr;
    }

    void *Header::sectionStringSectionData() {
        return this->sectionData(this->shstrndx);
    }

    void *Header::runtimeSectionStringSectionData() {
        return this->runtimeSectionData(this->shstrndx);
    }

    char *Header::lookupString(uint32_t section, uint32_t offset) {
        return &((char *)this->sectionData(section))[offset];
    }

    char *Header::runtimeLookupString(uint32_t section, uint32_t offset) {
        return &((char *)this->runtimeSectionData(section))[offset];
    }

    char *Header::lookupSectionString(uint32_t offset) {
        return this->lookupString(this->shstrndx, offset);
    }

    char *Header::runtimeLookupSectionString(uint32_t offset) {
        return this->runtimeLookupString(this->shstrndx, offset);
    }

    uint32_t Header::sectionByType(word_t type, uint32_t base) {
        for(base; this->sectionHeader(base)->type != type && base < this->shnum; base ++) {
            // Pass
        }

        if(this->sectionHeader(base)->type == type) {
            return base;
        }

        return 0;
    }

    Symbol *Header::symbol(uint32_t section, uint32_t offset) {
        SectionHeader *sect = this->sectionHeader(section);
        Symbol *symbols = (Symbol *)this->sectionData(section);

        if(offset >= sect->entries()) {
            return 0;
        }

        return (Symbol *)((addr_logical_t)symbols + offset * sect->entsize);
    }

    Symbol *Header::runtimeSymbol(uint32_t section, uint32_t offset) {
        SectionHeader *sect = this->sectionHeader(section);
        Symbol *symbols = (Symbol *)this->runtimeSectionData(section);

        if(offset >= sect->entries()) {
            return 0;
        }

        return (Symbol *)((addr_logical_t)symbols + offset * sect->entsize);
    }

    char *Header::symbolName(uint32_t section, uint32_t offset) {
        SectionHeader *sect = this->sectionHeader(section);
        Symbol *symbol = this->symbol(section, offset);

        return this->lookupString(sect->link, symbol->name);
    }

    char *Header::runtimeSymbolName(uint32_t section, uint32_t offset) {
        SectionHeader *sect = this->sectionHeader(section);
        Symbol *symbol = this->runtimeSymbol(section, offset);

        return this->runtimeLookupString(sect->link, symbol->name);
    }

    uint32_t Header::runtimeFindSymbolId(uint32_t addr, word_t type) {
        uint32_t section_id = this->sectionByType(SHT_SYMTAB, 0);
        SectionHeader *symtab = this->sectionHeader(section_id);
        Symbol *best = NULL;
        uint32_t best_id = 0;
        uint32_t delta = 0xffffffff;

        for(uint32_t i = 0; i < symtab->entries(); i ++) {
            Symbol *curr = this->runtimeSymbol(section_id, i);

            if(curr->info != type) {
                continue;
            }

            if(curr->value <= addr && ((addr - curr->value) < delta)) {
                best = curr;
                best_id = i;
                delta = addr - curr->value;
            }
        }

        return best_id;
    }

    Symbol *Header::runtimeFindSymbol(uint32_t addr, word_t type) {
        uint32_t section_id = this->sectionByType(SHT_SYMTAB, 0);

        return this->runtimeSymbol(section_id, this->runtimeFindSymbolId(addr, type));
    }

    char *Header::runtimeFindSymbolName(uint32_t addr, word_t type) {
        uint32_t section_id = this->sectionByType(SHT_SYMTAB, 0);

        return this->runtimeSymbolName(section_id, this->runtimeFindSymbolId(addr, type));
    }

    uint32_t SectionHeader::entries() {
        return this->size / this->entsize;
    }

    uint8_t st_bind(uint8_t x) {return x << 4;}
    uint8_t st_type(uint8_t x) {return x & 0xf;}
    uint8_t st_info(uint8_t b, uint8_t t) {return st_bind(b) + st_type(t);}

    void load_kernel_elf(uint32_t num, uint32_t size, addr_logical_t addr, uint32_t shndx) {
        kernel_elf = (Header *)kmalloc((num * size) + sizeof(Header), 0);

        kernel_elf->shnum = num;
        kernel_elf->shentsize = size;
        kernel_elf->shoff = sizeof(Header);
        kernel_elf->shstrndx = shndx;

        _memcpy((void *)((addr_logical_t)kernel_elf + sizeof(Header)), (void *)addr, (num * size));

        // Now go through all the headers and update their addresses if needed
        for(uint32_t i = 0; i < num; i ++) {
            SectionHeader *hdr = kernel_elf->sectionHeader(i);

            if(hdr->addr && hdr->addr < KERNEL_VM_BASE) {
                hdr->addr += KERNEL_VM_BASE;
            }

        }
    }

    Header *kernel_elf;
}

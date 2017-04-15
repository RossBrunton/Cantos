#ifndef _HPP_ELF_MUTEX_
#define _HPP_ELF_MUTEX_

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

namespace elf {
    typedef uint32_t addr_t;
    typedef uint16_t half_t;
    typedef uint32_t off_t;
    typedef int32_t sword_t;
    typedef uint32_t word_t;

    const uint32_t EI_NIDENT = 16;

    struct SectionHeader;
    struct Symbol;
    struct Header {
        uint8_t ident[EI_NIDENT];
        half_t type;
        half_t machine;
        word_t version;
        addr_t entry;
        off_t phoff;
        off_t shoff;
        word_t flags;
        half_t ehsize;
        half_t phentsize;
        half_t phnum;
        half_t shentsize;
        half_t shnum;
        half_t shstrndx;

        char *lookupString(uint32_t section, uint32_t offset);
        char *runtimeLookupString(uint32_t section, uint32_t offset);
        uint32_t sectionByType(word_t type, uint32_t base);

        SectionHeader *sectionHeader(uint32_t id);
        void *sectionData(uint32_t id);
        void *runtimeSectionData(uint32_t id);

        SectionHeader *sectionStringSection();
        void *sectionStringSectionData();
        void *runtimeSectionStringSectionData();
        char *lookupSectionString(uint32_t offset);
        char *runtimeLookupSectionString(uint32_t offset);

        Symbol *symbol(uint32_t section, uint32_t offset);
        Symbol *runtimeSymbol(uint32_t section, uint32_t offset);
        char *symbolName(uint32_t section, uint32_t offset);
        char *runtimeSymbolName(uint32_t section, uint32_t offset);

        uint32_t runtimeFindSymbolId(uint32_t addr, word_t type);
        Symbol *runtimeFindSymbol(uint32_t addr, word_t type);
        char *runtimeFindSymbolName(uint32_t addr, word_t type);
    };

    const uint8_t EI_MAG0 = 0;
    const uint8_t EI_MAG1 = 1;
    const uint8_t EI_MAG2 = 2;
    const uint8_t EI_MAG3 = 3;
    const uint8_t EI_CLASS = 4;
    const uint8_t EI_DATA = 5;
    const uint8_t EI_VERSION = 6;
    const uint8_t EI_PAD = 7;

    const uint8_t ELFMAG0 = 0x7f;
    const uint8_t ELFMAG1 = 'E';
    const uint8_t ELFMAG2 = 'L';
    const uint8_t ELFMAG3 = 'F';
    const uint8_t ELFCLASSNONE = 0;
    const uint8_t ELFCLASS32 = 1;
    const uint8_t ELFCLASS64 = 2;
    const uint8_t ELFDATANONE = 0;
    const uint8_t ELFDATA2LSB = 1;
    const uint8_t ELFDATA2MSB = 2;


    const half_t ET_NONE = 0;
    const half_t ET_REL = 1;
    const half_t ET_EXEC = 2;
    const half_t ET_DYN = 3;
    const half_t ET_CORE = 4;
    const half_t ET_LOPROC = 0xff00;
    const half_t ET_HIPROC = 0xffff;

    const half_t EM_NONE = 0;
    const half_t EM_M32 = 1;
    const half_t EM_SPARC = 2;
    const half_t EM_386 = 3;
    const half_t EM_68K = 4;
    const half_t EM_88K = 5;
    const half_t EM_860 = 7;
    const half_t EM_MIPS = 8;

    const word_t EV_NONE = 0;
    const word_t EV_CURRENT = 1;

    struct SectionHeader {
        word_t name;
        word_t type;
        word_t flags;
        addr_t addr;
        off_t offset;
        word_t size;
        word_t link;
        word_t info;
        word_t addralign;
        word_t entsize;

        uint32_t entries();
    };

    const word_t SHN_UNDEF = 0;
    const word_t SHN_LORESERVE = 0xff00;
    const word_t SHN_LOPROC = 0xff00;
    const word_t SHN_HIPROC = 0xff1f;
    const word_t SHN_ABS = 0xfff1;
    const word_t SHN_COMMON = 0xfff2;
    const word_t SHN_HIRESERVE = 0xffff;

    const word_t SHT_NULL = 0;
    const word_t SHT_PROGBITS = 1;
    const word_t SHT_SYMTAB = 2;
    const word_t SHT_STRTAB = 3;
    const word_t SHT_RELA = 4;
    const word_t SHT_HASH = 5;
    const word_t SHT_DYNAMIC = 6;
    const word_t SHT_NOTE = 7;
    const word_t SHT_NOBITS = 8;
    const word_t SHT_REL = 9;
    const word_t SHT_SHLIB = 10;
    const word_t SHT_DYNSYM = 11;
    const word_t SHT_LOPROC = 0x70000000;
    const word_t SHT_HIPROC = 0x7fffffff;
    const word_t SHT_LOUSER = 0x80000000;
    const word_t SHT_HIUSER = 0xffffffff;

    const word_t SHF_WRITE = 1;
    const word_t SHF_ALLOC = 2;
    const word_t SHF_EXECINSTR = 4;
    const word_t SHF_MASKPROC = 0xf0000000;

    struct Symbol {
        word_t name;
        addr_t value;
        word_t size;
        uint8_t info;
        uint8_t other;
        half_t shndx;
    };

    uint8_t st_bind(uint8_t x);
    uint8_t st_type(uint8_t x);
    uint8_t st_info(uint8_t b, uint8_t t);

    const word_t STB_LOCAL = 0;
    const word_t STB_GLOBAL = 1;
    const word_t STB_WEAK = 2;
    const word_t STB_LOPROC = 13;
    const word_t STB_HIPROC = 15;

    const word_t STT_NOTYPE = 0;
    const word_t STT_OBJECT = 1;
    const word_t STT_FUNC = 2;
    const word_t STT_SECTION = 3;
    const word_t STT_FILE = 4;
    const word_t STT_LOPROC = 13;
    const word_t STT_HIPROC = 15;

    extern Header *kernel_elf;
    void load_kernel_elf(uint32_t num, uint32_t size, uint32_t addr, uint32_t shndx);
}

#endif

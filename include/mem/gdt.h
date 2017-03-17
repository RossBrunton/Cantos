#ifndef _H_MEM_GDT_
#define _H_MEM_GDT_

#include <stdint.h>

typedef struct __attribute__((packed)) gdt_descriptor_e {
    uint16_t size;
    uint32_t offset;
} gdt_descriptor_t;

typedef struct gdt_table_entry_e {
    uint64_t value;
} gdt_table_entry_t;

#define GDT_ACCESS_PRESENT (1<<7)
#define GDT_ACCESS_PRIV(x) ((x)<<5)
#define GDT_ACCESS_EXECUTABLE (1<<3)
#define GDT_ACCESS_DC (1<<2)
#define GDT_ACCESS_RW (1<<1)

#define GDT_FLAG_GRANULARITY (1<<3)
#define GDT_FLAG_SIZE (1<<2)

#define GDT_CODE_OFFSET 0x10
#define GDT_DATA_OFFSET 0x08

#define GDT_SELECTOR(rpl, ti, index) ((rpl) | ((ti) << 2) | ((index) << 3))

void gdt_set_entry(gdt_table_entry_t *entry, uint32_t base, uint32_t limit, uint8_t flags, uint8_t access);
void gdt_init();
void gdt_setup();

#endif

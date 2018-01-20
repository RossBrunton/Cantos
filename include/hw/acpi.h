#ifndef _H_IO_ACPI_
#define _H_IO_ACPI_

#include "main/common.hpp"


namespace acpi {

#define ACPI_RSDP_SIG_A 0x20445352 // " DSR"
#define ACPI_RSDP_SIG_B 0x20525450 // " RTP"

typedef struct acpi_sdt_header {
    uint32_t signature;
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} acpi_sdt_header_t;

typedef struct acpi_rsdt {
    acpi_sdt_header_t h;
    acpi_sdt_header_t *first;
} acpi_rsdt_t;

typedef struct acpi_rsdp_descriptor {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    acpi_rsdt_t *rsdt;

    // The following only if revision >= 2
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t checksum_extended;
    uint8_t reserved[3];
} __attribute__ ((packed)) acpi_rsdp_descriptor_t;


// MADT
#define ACPI_MADT_SIG 0x43495041 // "CIPA"
#define ACPI_MADT_COPY MAX_CORES
#define ACPI_MADT_PROC 0x0
#define ACPI_MADT_IOAPIC 0x1
#define ACPI_MADT_ISO 0x2

typedef struct acpi_madt_proc {
    uint8_t id;
    uint8_t apic_id;
    uint32_t flags;
} acpi_madt_proc_t;

typedef struct acpi_madt_ioapic {
    uint8_t id;
    uint8_t reserved;
    addr_phys_t addr;
    addr_phys_t int_base;
} acpi_madt_ioapic_t;

typedef struct acpi_madt_iso {
    uint8_t bus_source;
    uint8_t irq_source;
    uint32_t global_int;
    uint16_t flags;
} acpi_madt_iso_t;

typedef struct acpi_madt {
    acpi_sdt_header_t h;
    uint32_t local_controller_address;
    uint32_t flags;
} __attribute__ ((packed)) acpi_madt_t;

typedef struct acpi_madt_entry {
    uint8_t type;
    uint8_t length;

    union {
        acpi_madt_proc_t proc;
        acpi_madt_ioapic_t ioapic;
        acpi_madt_iso_t iso;
    } dat;
} __attribute__ ((packed)) acpi_madt_entry_t;


void low_acpi_setup();

// Data to be used
extern acpi_madt_proc_t acpi_procs[ACPI_MADT_COPY];
extern acpi_madt_ioapic_t acpi_ioapics[ACPI_MADT_COPY];
extern acpi_madt_iso_t acpi_isos[ACPI_MADT_COPY];

extern uint32_t acpi_proc_count;
extern uint32_t acpi_ioapic_count;
extern uint32_t acpi_iso_count;

extern uint32_t acpi_lapic_base;

}

#endif

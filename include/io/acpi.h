#ifndef _H_IO_ACPI_
#define _H_IO_ACPI_

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
} __attribute__ ((packed)) acpi_rsdp_descriptor_t;

typedef struct acpi_rsdp_descriptor_extended {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    acpi_rsdt_t *rsdt;
    
    // Extended part
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t checksum_extended;
    uint8_t reserved[3];
} __attribute__ ((packed)) acpi_rsdp_descriptor_extended_t;

void low_acpi_setup();

#endif

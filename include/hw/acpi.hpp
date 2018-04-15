#pragma once

#include "main/common.hpp"

/** Contains methods for parsing the acpi headers
 *
 * At the current time, the only things parsed out of the tables are the madt tables.
 *
 * To populate the various structures in this module, @ref low_setup must be called from low memory, which will
 *  scan for the various tables, and then fill in the high memory versions of them. That is, low_setup must be called
 *  before @ref kernel_main, but its structures must be used after that point.
 *
 * Please note that any pointers you find in the copied structure will likely be invalid.
 */
namespace acpi {

/** Header field of all ACPI SDT tables
 *
 * All tables (except the RSDP) contain this header at their start.
 */
struct SdtHeader {
    uint32_t signature; /**< The signature for this sdt, each table type has a unique id */
    uint32_t length; /**< Length of the table, including this header */
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
};

/** The RSDT table
 *
 * Contains an array of ACPI table addresses, up until h.length bytes worth of them have been consumed.
 *
 * Each entry is 32 bits if a RDST pointer was followed, or 64 bits if a XDST pointer was.
 *
 * @TODO Different structures for RSDT/XSDT.
 */
struct Rsdt {
    SdtHeader h;
    SdtHeader* first; /**< This is the first entry, others follow this */
};

/** The RSDP descriptior
 *
 * This basically contains various information about the RSDT structure, and exists somewhere in memory, where it
 *  must be found.
 */
struct RsdpDesc {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    Rsdt* rsdt;

    // The following only if revision >= 2
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t checksum_extended;
    uint8_t reserved[3];
} __attribute__((packed));
/** The first word of the RSDP's signature */
const uint32_t RSDP_SIG_A = 0x20445352; // " DSR"
/** The second word of the RSDP's signature */
const uint32_t RSDP_SIG_B = 0x20525450; // " RTP"


// MADT
/** MADT signature */
const uint32_t MADT_SIG = 0x43495041; // "CIPA"
/** Number of entries to create for each of the proc, ioapic and iso tables
 *
 * This essentially limits each of them to a maximum of this number of objects. This defaults to the MAX_CORES
 *  configuration variable.
 */
const size_t MADT_COPY = MAX_CORES;

/** The types of Madt entries */
enum class MadtType : uint8_t { PROC = 0x0, IOAPIC = 0x1, ISO = 0x2 };

/** Information about a processor from a MADT entry */
struct MadtProc {
    uint8_t id;
    uint8_t apic_id;
    uint32_t flags;
};

/** Information about an ioapic from a MADT entry */
struct MadtIoapic {
    uint8_t id;
    uint8_t reserved;
    addr_phys_t addr;
    addr_phys_t int_base;
};

/** Information about an interrupt service override from a MADT entry */
struct MadtIso {
    uint8_t bus_source;
    uint8_t irq_source;
    uint32_t global_int;
    uint16_t flags;
};

/** The MADT table
 *
 * This contains information about the processors, ioapics and interrupt service overrides installed in the system.
 *
 * Entries are stored in memory (from the source) directly after this, for the rest of the table.
 *
 * Entries in this table are copied into arrays (@ref procs, @ref ioapics and @ref isos) during @ref low_setup.
 */
struct Madt {
    SdtHeader h;
    uint32_t local_controller_address;
    uint32_t flags;
} __attribute__((packed));

/** A single entry in the MADT table
 *
 * It's type (and length) depends on the type of MADT entry it is.
 */
struct MadtEntry {
    MadtType type;
    uint8_t length;

    union {
        MadtProc proc;
        MadtIoapic ioapic;
        MadtIso iso;
    } dat;
} __attribute__((packed));

/** Searches low memory for the ACPI tables, and copies them into kernel owned space
 *
 * This basically scans through the places where ACPI tables can live and finds one (calling @ref low_error if it fails
 *  to find one). It then parses the tables and stores their information in various structures and arrays in the acpi
 *  namespace.
 *
 * This function must be called from low memory, does not use any memory allocation and is not thread safe.
 */
void low_setup();

/** Copies of all the MADT entries for processors
 *
 * This will be @ref proc_count entries long.
 */
extern MadtProc procs[MADT_COPY];
/** Copies of all the MADT entries for ioapics
 *
 * This will be @ref ioapic_count entries long.
 */
extern MadtIoapic ioapics[MADT_COPY];
/** Copies of all the MADT entries for interupt service overrides
 *
 * This will be @ref iso_count entries long.
 */
extern MadtIso isos[MADT_COPY];

/** Number of MADT processor entries (also the number of processors in the system) */
extern uint32_t proc_count;
/** Number of MADT ioapic entries (also the number of ioapics in the system) */
extern uint32_t ioapic_count;
/** Number of MADT iso entries (also the number of isos in the system) */
extern uint32_t iso_count;

/** The base address of LAPIC memory as defined in the tables */
extern addr_phys_t lapic_base;
}

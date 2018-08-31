#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hw/acpi.hpp"
#include "main/common.hpp"

namespace acpi {
MadtProc procs[MADT_COPY];
MadtIoapic ioapics[MADT_COPY];
MadtIso isos[MADT_COPY];

uint32_t proc_count;
uint32_t ioapic_count;
uint32_t iso_count;

addr_phys_t lapic_base;
bool acpi_found;
}

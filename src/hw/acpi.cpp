#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hw/acpi.h"
#include "main/common.hpp"

namespace acpi {
    acpi_madt_proc_t acpi_procs[ACPI_MADT_COPY];
    acpi_madt_ioapic_t acpi_ioapics[ACPI_MADT_COPY];
    acpi_madt_iso_t acpi_isos[ACPI_MADT_COPY];

    uint32_t acpi_proc_count;
    uint32_t acpi_ioapic_count;
    uint32_t acpi_iso_count;

    addr_phys_t acpi_lapic_base;
}

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hw/acpi.h"
#include "main/common.hpp"

namespace acpi {
    acpi_mdat_proc_t acpi_procs[ACPI_MDAT_COPY];
    acpi_mdat_ioapic_t acpi_ioapics[ACPI_MDAT_COPY];
    acpi_mdat_iso_t acpi_isos[ACPI_MDAT_COPY];

    uint32_t acpi_proc_count;
    uint32_t acpi_ioapic_count;
    uint32_t acpi_iso_count;

    addr_phys_t acpi_lapic_base;
}

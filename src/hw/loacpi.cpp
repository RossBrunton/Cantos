#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hw/acpi.hpp"
#include "main/common.hpp"
#include "main/loerror.hpp"

namespace acpi {
static void* _memcpy(void* destination, const void* source, size_t num) {
    size_t i;
    for (i = 0; i < num; i++) {
        ((char*)destination)[i] = ((char*)source)[i];
    }
    return destination;
}

void low_setup() {
    addr_phys_t ptr;
    addr_phys_t top;
    RsdpDesc* rsdp = NULL;
    Rsdt* rsdt;
    addr_phys_t hdr;

    // Search EBDA
    ptr = ((*(uint32_t*)(0x40e) << 4) & 0x000fffff);
    ptr += ptr % 16;
    top = ptr + 1024;

    for (; ptr < top; ptr += 16) {
        if (*(uint32_t*)ptr == RSDP_SIG_A && *(uint32_t*)ptr + 1 == RSDP_SIG_B) {
            rsdp = (RsdpDesc*)ptr;
        }
    }

    // Search main bios area
    ptr = 0x000e0000;
    top = 0x000fffff;

    for (; ptr < top; ptr += 16) {
        if (*(uint32_t*)ptr == RSDP_SIG_A && *(uint32_t*)(ptr + 4) == RSDP_SIG_B) {
            rsdp = (RsdpDesc*)ptr;
        }
    }

    if (!rsdp) {
        // Fail if we can't find it
        LOW(bool, acpi_found) = false;
        LOW(uint32_t, proc_count) = 1;
        LOW(uint32_t, ioapic_count) = 1;
        LOW(uint32_t, iso_count) = 0;
        return;
    }


    // Found it, now traverse the table

    // Loop through all tables
    bool using_xsdt = false;
    if (rsdp->revision >= 2) {
        using_xsdt = true;
        rsdt = (Rsdt*)(rsdp->xsdt_addr);
    } else {
        rsdt = rsdp->rsdt;
    }

    hdr = (addr_phys_t)&rsdt->first;

    bool found_madt = false;
    for (; (addr_phys_t)hdr < (addr_phys_t)rsdt + rsdt->h.length; hdr += using_xsdt ? 8 : 4) {
        SdtHeader* hdata = *(SdtHeader**)hdr;

        switch (hdata->signature) {
        case MADT_SIG: {
            (void)0;
            // Copy data! :D
            int pc = 0;
            int ic = 0;
            int sc = 0;
            Madt* madt = (Madt*)hdata;
            addr_phys_t p = (addr_phys_t)(madt + 1);

            found_madt = true;

            LOW(uint32_t, lapic_base) = madt->local_controller_address;

            while (p < (addr_phys_t)madt + madt->h.length) {
                MadtEntry* entry = (MadtEntry*)p;

                switch (entry->type) {
                case MadtType::PROC:
                    _memcpy(&(LOW(MadtProc*, procs[pc])), &(entry->dat), sizeof(MadtProc));
                    pc++;
                    break;

                case MadtType::IOAPIC:
                    _memcpy(&(LOW(MadtIoapic*, ioapics[ic])), &(entry->dat), sizeof(MadtIoapic));
                    ic++;
                    break;

                case MadtType::ISO:
                    _memcpy(&(LOW(MadtIso*, isos[sc])), &(entry->dat), sizeof(MadtIso));
                    sc++;
                    break;
                }

                p += entry->length;
            }

            if (!pc)
                low_error("ACPI: MADT table doesn't specify any processors");
            if (!ic)
                low_error("ACPI: MADT table doesn't specify any ioapics");
            if (!sc)
                low_error("ACPI: MADT table doesn't specify any interrupt source overrides");

            LOW(uint32_t, proc_count) = pc;
            LOW(uint32_t, ioapic_count) = ic;
            LOW(uint32_t, iso_count) = sc;
            LOW(bool, acpi_found) = true;

            break;
        }
        }
    }

    if (!found_madt)
        low_error("ACPI: Didn't find a MADT table");
}
}

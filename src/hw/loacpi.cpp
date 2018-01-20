#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hw/acpi.h"
#include "main/common.hpp"
#include "main/loerror.hpp"

namespace acpi {
acpi_madt_t low_madt_hold[ACPI_MADT_COPY];

static void *_memcpy(void *destination, const void *source, size_t num) {
    size_t i;
    for(i = 0; i < num; i ++) {
        ((char *)destination)[i] = ((char *)source)[i];
    }
    return destination;
}

void low_acpi_setup() {
    addr_phys_t ptr;
    addr_phys_t top;
    acpi_rsdp_descriptor_t *rsdp = NULL;
    acpi_rsdt_t* rsdt;
    addr_phys_t hdr;

    // Search EBDA
    ptr = ((*(uint32_t *)(0x40e) << 4) & 0x000fffff);
    ptr += ptr % 16;
    top = ptr + 1024;

    for(; ptr < top; ptr += 16) {
        if(*(uint32_t *)ptr == ACPI_RSDP_SIG_A && *(uint32_t *)ptr + 1 == ACPI_RSDP_SIG_B) {
            rsdp = (acpi_rsdp_descriptor_t *)ptr;
        }
    }

    // Search main bios area
    ptr = 0x000e0000;
    top = 0x000fffff;

    for(; ptr < top; ptr += 16) {
        if(*(uint32_t *)ptr == ACPI_RSDP_SIG_A && *(uint32_t *)(ptr + 4) == ACPI_RSDP_SIG_B) {
            rsdp = (acpi_rsdp_descriptor_t *)ptr;
        }
    }

    if(!rsdp) {
        // Fail in some way if we can't find it
        low_error("ACPI: Failed to find rsdp descriptior");
    }


    // Found it, now traverse the table

    // Loop through all tables
    bool using_xsdt = false;
    if(rsdp->revision >= 2) {
        using_xsdt = true;
        rsdt = (acpi_rsdt_t *)(rsdp->xsdt_addr);
    }else{
        rsdt = rsdp->rsdt;
    }

    hdr = (addr_phys_t)&rsdt->first;

    bool found_madt = false;
    for(; (addr_phys_t)hdr < (addr_phys_t)rsdt + rsdt->h.length; hdr += using_xsdt ? 8 : 4) {
        acpi_sdt_header_t *hdata = *(acpi_sdt_header_t **)hdr;

        switch(hdata->signature) {
            case ACPI_MADT_SIG: {
                (void)0;
                // Copy data! :D
                int pc = 0;
                int ic = 0;
                int sc = 0;
                acpi_madt_t *madt = (acpi_madt_t *)hdata;
                addr_phys_t p = (addr_phys_t)(madt+1);

                found_madt = true;

                LOW(uint32_t, acpi_lapic_base) = madt->local_controller_address;

                while(p < (addr_phys_t)madt + madt->h.length) {
                    acpi_madt_entry_t *entry = (acpi_madt_entry_t *)p;

                    switch(entry->type) {
                        case ACPI_MADT_PROC:
                            _memcpy(
                                &(LOW(acpi_madt_proc_t *, acpi_procs[pc])),
                                &(entry->dat), sizeof(acpi_madt_proc_t));
                            pc ++;
                            break;

                        case ACPI_MADT_IOAPIC:
                            _memcpy(
                                &(LOW(acpi_madt_ioapic_t*, acpi_ioapics[ic])),
                                &(entry->dat), sizeof(acpi_madt_ioapic_t));
                            ic ++;
                            break;

                        case ACPI_MADT_ISO:
                            _memcpy(
                                &(LOW(acpi_madt_iso_t*, acpi_isos[sc])),
                                &(entry->dat), sizeof(acpi_madt_iso_t));
                            sc ++;
                            break;
                    }

                    p += entry->length;
                }

                if(!pc) low_error("ACPI: MADT table doesn't specify any processors");
                if(!ic) low_error("ACPI: MADT table doesn't specify any ioapics");
                if(!sc) low_error("ACPI: MADT table doesn't specify any interrupt source overrides");

                LOW(uint32_t, acpi_proc_count) = pc;
                LOW(uint32_t, acpi_ioapic_count) = ic;
                LOW(uint32_t, acpi_iso_count) = sc;

                break;
            }
        }
    }

    if(!found_madt) low_error("ACPI: Didn't find a MADT table");
}
}

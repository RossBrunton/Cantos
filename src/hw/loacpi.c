#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hw/acpi.h"
#include "main/common.h"

acpi_mdat_t low_mdat_hold[ACPI_MDAT_COPY];

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
    acpi_rsdp_descriptor_extended_t *rsdp = NULL;
    acpi_rsdt_t* rsdt;
    addr_phys_t hdr;
    
    // Search EBDA
    ptr = ((*(uint32_t *)(0x40e) << 4) & 0x000fffff);
    ptr += ptr % 16;
    top = ptr + 1024;
    
    for(; ptr < top; ptr += 16) {
        if(*(uint32_t *)ptr == ACPI_RSDP_SIG_A && *(uint32_t *)ptr + 1 == ACPI_RSDP_SIG_B) {
            rsdp = (acpi_rsdp_descriptor_extended_t *)ptr;
        }
    }
    
    // Search main bios area
    ptr = 0x000e0000;
    top = 0x000fffff;
    
    for(; ptr < top; ptr += 16) {
        if(*(uint32_t *)ptr == ACPI_RSDP_SIG_A && *(uint32_t *)(ptr + 4) == ACPI_RSDP_SIG_B) {
            rsdp = (acpi_rsdp_descriptor_extended_t *)ptr;
        }
    }
    
    if(!rsdp) {
        // Fail in some way if we can't find it
        while(1) {}
    }
    
    
    // Found it, now traverse the table
    
    // Loop through all tables
    rsdt = rsdp->rsdt;
    hdr = (addr_phys_t)&rsdt->first;
    
    for(; (addr_phys_t)hdr < (addr_phys_t)rsdt + rsdt->h.length; hdr += 8) {
        acpi_sdt_header_t *hdata = *(acpi_sdt_header_t **)hdr;
        
        switch(hdata->signature) {
            case ACPI_MDAT_SIG:
                (void)0;
                // Copy data! :D
                int pc = 0;
                int ic = 0;
                int sc = 0;
                acpi_mdat_t *mdat = (acpi_mdat_t *)hdata;
                addr_phys_t p = (addr_phys_t)(mdat+1);
                
                ACPI_LOW(uint32_t, acpi_lapic_base) = mdat->local_controller_address;
                
                while(p < (addr_phys_t)mdat + mdat->h.length) {
                    acpi_mdat_entry_t *entry = (acpi_mdat_entry_t *)p;
                    
                    switch(entry->type) {
                        case ACPI_MDAT_PROC:
                            _memcpy(
                                &(ACPI_LOW(acpi_mdat_proc_t *, acpi_procs[pc])),
                                &(entry->dat), sizeof(acpi_mdat_proc_t));
                            pc ++;
                            break;
                        
                        case ACPI_MDAT_IOAPIC:
                            _memcpy(
                                &(ACPI_LOW(acpi_mdat_ioapic_t*, acpi_ioapics[ic])),
                                &(entry->dat), sizeof(acpi_mdat_ioapic_t));
                            ic ++;
                            break;
                        
                        case ACPI_MDAT_ISO:
                            _memcpy(
                                &(ACPI_LOW(acpi_mdat_iso_t*, acpi_isos[sc])),
                                &(entry->dat), sizeof(acpi_mdat_iso_t));
                            sc ++;
                            break;
                    }
                    
                    p += entry->length;
                }
                
                ACPI_LOW(uint32_t, acpi_proc_count) = pc;
                ACPI_LOW(uint32_t, acpi_ioapic_count) = ic;
                ACPI_LOW(uint32_t, acpi_iso_count) = sc;
                
                break;
        }
    }
    
    
}

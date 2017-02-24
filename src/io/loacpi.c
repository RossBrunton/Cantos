#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "io/acpi.h"
#include "main/common.h"

void low_acpi_setup() {
    addr_phys_t ptr;
    addr_phys_t top;
    acpi_rsdp_descriptor_extended_t *rsdp = NULL;
    acpi_rsdt_t* rsdt;
    acpi_sdt_header_t* hdr;
    
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
    
    // Loop through all tables
    rsdt = rsdp->rsdt;
    hdr = rsdt->first;
    for(hdr = rsdt->first; (addr_phys_t)hdr < (addr_phys_t)rsdt + rsdt->h.length; hdr ++) {
        switch(hdr->signature) {
            // None yet!
        }
    }
    
    if(!rsdp) {
        // Fail in some way
        while(1) {}
    }
}

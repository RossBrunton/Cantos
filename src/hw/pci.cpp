#include <stdint.h>

#include "hw/pci.hpp"
#include "main/printk.hpp"
#include "int/ioapic.hpp"
#include "int/lapic.hpp"
#include "main/panic.hpp"
#include "structures/mutex.hpp"

extern "C" {
    #include "hw/ports.h"
    #include "hw/utils.h"
    #include "int/numbers.h"
}

namespace pci {
    Device *devices = NULL;

    static uint32_t _read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
        uint32_t addr;

        if(offset % 0x4) {
            panic("Unaligned access in PCI bus.");
        }

        addr = (bus << 16) | (slot << 11) | (func << 8) | offset | 0x80000000;

        outl(IO_PORT_PCI_ADDRESS, addr);

        return inl(IO_PORT_PCI_DATA);
    }

    static uint32_t _read_device_vendor(uint8_t bus, uint8_t slot, uint8_t fn) {
        return _read(bus, slot, fn, 0);
    }



    Device::Device(uint8_t bus, uint8_t slot) {
        this->bus = bus;
        this->slot = slot;

        uint32_t first = this->get32(0, VENDOR_ID);
        this->vendor_id = first & 0xffff;
        this->device_id = first >> 16;

        uint32_t second = this->get32(0, REVISION);
        this->revision = second & 0xff;
        this->prog_if = (second >> 8) & 0xff;
        this->device_subclass = (second >> 16) & 0xff;
        this->device_class = (second >> 24) & 0xff;

        uint8_t ht = this->get8(0, HEADER_TYPE);
        this->header_type = ht & 0x7f;
        this->multifunction = ht & 0x80;
    }

    uint8_t Device::get8(uint8_t fn, uint8_t addr) {
        uint8_t base = addr & 0xfc;
        uint8_t offset = addr % 0x4;

        uint32_t result = this->get32(fn, base);
        return (result >> offset * 8) & 0xff;
    }

    uint16_t Device::get16(uint8_t fn, uint8_t addr) {
        uint8_t base = addr & 0xfc;
        uint8_t offset = addr % 0x4;

        uint32_t result = this->get32(fn, base);
        return (result >> offset * 8) & 0xffff;
    }

    uint32_t Device::get32(uint8_t fn, uint8_t addr) {
        return _read(this->bus, this->slot, fn, addr);
    }

    static void _search_bus(uint8_t bus, uint8_t start) {
        for(uint16_t i = start; i <= 32; i ++) {
            if(_read_device_vendor(bus, i, 0) != 0xffffffff) {
                Device *device = new Device(bus, i);
                device->next = devices;
                devices = device;

                // Check if it is a pci to pci bridge
                if(device->device_class == 0x06 && device->device_subclass == 0x04) {
                    printk("Found bridge!\n");
                    _search_bus(device->get8(0, PBR_SECONDARY_BUS_NUM), 0);
                }
            }
        }
    }

    void init() {
        devices = new Device(0, 0);
        devices->next = NULL;

        // Seach devices
        if(devices->multifunction) {
            for(uint8_t fn = 0; fn < 8; fn ++) {
                if(_read_device_vendor(0, 0, fn) != 0xffffffff) break;
                _search_bus(fn, fn == 0 ? 1 : 0);
            }
        }else{
            _search_bus(0, 1);
        }

        // Lets list them
        Device *device = devices;
        do {
            printk("PCI Device %x:%x.%x is %x:%x\n", device->bus, device->slot, device->prog_if, device->vendor_id, device->device_id);
        } while((device = device->next));
    }
}

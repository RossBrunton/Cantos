#include <stdint.h>

#include "hw/pci.hpp"
#include "main/printk.hpp"
#include "int/ioapic.hpp"
#include "int/lapic.hpp"
#include "main/panic.hpp"
#include "structures/mutex.hpp"
#include "structures/list.hpp"

extern "C" {
    #include "hw/ports.h"
    #include "hw/utils.h"
    #include "int/numbers.h"
}

namespace pci {
    list<Device> devices;

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



    Device::Device(uint8_t bus, uint8_t slot, uint8_t function) : bus(bus), slot(slot), function(function) {
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
        for(uint16_t i = start; i < 32; i ++) {
            if(_read_device_vendor(bus, i, 0) != 0xffffffff) {
                Device& device = (devices.emplace_back(bus, i, 0), devices.back());

                // Check if it is a pci to pci bridge
                if(device.device_class == 0x06 && device.device_subclass == 0x04) {
#if DEBUG_PCI
                    printk("Found PCI bridge!\n");
#endif
                    _search_bus(device.get8(0, PBR_SECONDARY_BUS_NUM), 0);
                }

                // Search for other functions
                if(device.multifunction) {
                    for(uint8_t j = 1; j < 8; j ++) {
                        if(_read_device_vendor(bus, i, 0) != 0xffffffff) {
                            Device& device = (devices.emplace_back(bus, i, j), devices.back());

                            // And check THIS for being a PCI to PCI bus
                            if(device.device_class == 0x06 && device.device_subclass == 0x04) {
#if DEBUG_PCI
                                printk("Found PCI bridge!\n");
#endif
                                _search_bus(device.get8(0, PBR_SECONDARY_BUS_NUM), 0);
                            }
                        }
                    }
                }
            }
        }
    }

    void init() {
        Device& device = (devices.emplace_back(0, 0, 0), devices.back());

        // Seach devices
        if(device.multifunction) {
            for(uint8_t fn = 0; fn < 8; fn ++) {
                if(_read_device_vendor(0, 0, fn) != 0xffffffff) break;
                _search_bus(fn, fn == 0 ? 1 : 0);
            }
        }else{
            _search_bus(0, 1);
        }
    }

void print_devices() {
        for(const auto &device : devices) {
            if(device.driver) {
                printk("[%x:%x.%x] %s (%x:%x, %x:%x.%x)\n", device.bus, device.slot, device.function,
                    device.driver->device_name().to_string(), device.vendor_id, device.device_id,
                    device.device_class, device.device_subclass, device.prog_if);
            }else{
                printk("[%x:%x.%x] (unknown) (%x:%x - %x:%x.%x)\n", device.bus, device.slot, device.function,
                    device.vendor_id, device.device_id,
                    device.device_class, device.device_subclass, device.prog_if);
            }
        }
    }
}

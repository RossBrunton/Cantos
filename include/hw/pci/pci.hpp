#ifndef _HPP_HW_PCI_
#define _HPP_HW_PCI_

#include "main/common.hpp"
#include "structures/list.hpp"
#include "structures/unique_ptr.hpp"

namespace pci {
    const uint8_t VENDOR_ID = 0x00;
    const uint8_t DEVICE_ID = 0x02;
    const uint8_t COMMAND = 0x04;
    const uint8_t STATUS = 0x06;
    const uint8_t REVISION = 0x08;
    const uint8_t PROG_IF = 0x09;
    const uint8_t SUBCLASS = 0x0a;
    const uint8_t CLASS = 0x0b;
    const uint8_t CACHE_LINE_SIZE = 0x0c;
    const uint8_t LATENCY_TIMER = 0x0d;
    const uint8_t HEADER_TYPE = 0x0e;
    const uint8_t BIST = 0x0f;

    // General purpose device (type 0x00)
    const uint8_t TYPE_GENERAL = 0x00;
    const uint8_t BAR0 = 0x10;
    const uint8_t BAR1 = 0x14;
    const uint8_t BAR2 = 0x18;
    const uint8_t BAR3 = 0x1c;
    const uint8_t BAR4 = 0x20;
    const uint8_t BAR5 = 0x24;
    const uint8_t CARDBUS_CIS = 0x28;
    const uint8_t SUBSYS_VEND_ID = 0x2c;
    const uint8_t SUBSYS_ID = 0x2e;
    const uint8_t EXPANSION_ROM_BASE = 0x30;
    const uint8_t CAPABILITIES = 0x34;
    const uint8_t INTERRUPT_LINE = 0x3c;
    const uint8_t INTERRUPT_PIN = 0x3d;
    const uint8_t MIN_GRANT = 0x3e;
    const uint8_t MAX_LATENCY = 0x3f;

    // PCI to PCI bridge (type 0x01)
    const uint8_t TYPE_PBR = 0x01;
    const uint8_t PBR_BAR0 = 0x10;
    const uint8_t PBR_BAR1 = 0x14;
    const uint8_t PBR_PRIMARY_BUS_NUM = 0x18;
    const uint8_t PBR_SECONDARY_BUS_NUM = 0x19;
    const uint8_t PBR_SUB_BUS_NUM = 0x1a;
    const uint8_t PBR_SECONDARY_LATENCY_TIMER = 0x1b;
    const uint8_t PBR_IO_BASE = 0x1c;
    const uint8_t PBR_IO_LIMIT = 0x1d;
    const uint8_t PBR_SECONDARY_STATUS = 0x1e;
    const uint8_t PBR_MEMORY_BASE = 0x20;
    const uint8_t PBR_MEMORY_LIMIT = 0x22;
    const uint8_t PBR_PREFETCH_MEMORY_BASE = 0x24;
    const uint8_t PBR_PREFETCH_MEMORY_LIMIT = 0x26;
    const uint8_t PBR_PREFETCH_BASE_UPPER = 0x28;
    const uint8_t PBR_PREFETCH_LIMIT_UPPER = 0x2c;
    const uint8_t PBR_IO_BASE_UPPER = 0x30;
    const uint8_t PBR_IO_LIMIT_UPPER = 0x32;
    const uint8_t PBR_CAPABILITY_POINTER = 0x34;
    const uint8_t PBR_EXPANSION_ROM_BASE = 0x38;
    const uint8_t PBR_INTERRUPT_LINE = 0x3c;
    const uint8_t PBR_INTERRUPT_PIN = 0x3d;
    const uint8_t PBR_BRIDGE_CONTROL = 0x3e;

    // PCI to Cardbus bridge (type 0x02)
    const uint8_t TYPE_CBR = 0x02;

    class Driver;

    class Device {
    public:
        uint16_t device_id;
        uint16_t vendor_id;
        uint8_t device_class;
        uint8_t device_subclass;
        uint8_t prog_if;
        uint8_t revision;
        uint8_t header_type;
        uint8_t bus;
        uint8_t slot;
        uint8_t function;
        uint8_t multifunction;
        unique_ptr<Driver> driver;

        Device(uint8_t bus, uint8_t slot, uint8_t function);
        uint8_t get8(uint8_t fn, uint8_t addr);
        uint16_t get16(uint8_t fn, uint8_t addr);
        uint32_t get32(uint8_t fn, uint8_t addr);
        void set8(uint8_t fn, uint8_t addr, uint8_t val);
        void set16(uint8_t fn, uint8_t addr, uint16_t val);
        void set32(uint8_t fn, uint8_t addr, uint32_t val);
    };

    class Driver {
    public:
        virtual void configure() = 0;
        virtual void handle_interrupt() = 0;
        virtual Utf8 device_name() = 0;
        virtual Utf8 driver_name() = 0;

        Driver(Device& device) : device(device) {};
        virtual ~Driver() {};
    protected:
        Device& device;
    };

    class DriverFactory {
    public:
        virtual void create_driver(Device &device) = 0;
        virtual void search_for_device(const vector<Device> &devices) = 0;
        virtual ~DriverFactory() {};
    };

    void init();
    extern vector<Device> devices;
    void print_devices();

    vector<unique_ptr<DriverFactory>> &getDriverFactoryRegistry();
    template<class T> class RegisterDriverFactory {
    public:
        RegisterDriverFactory() {
            unique_ptr<DriverFactory> t = make_unique<T>();
            getDriverFactoryRegistry().push_back(move(t));
        }
    };
}

#endif

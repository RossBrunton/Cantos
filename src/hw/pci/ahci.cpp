#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "main/common.hpp"
#include "hw/pci/pci.hpp"

using namespace pci;

namespace pci_ahci {
    const uint8_t AHCI_CLASS = 0x01;
    const uint8_t AHCI_SUBCLASS = 0x06;

    struct hba_port_t {
        uint32_t clb; // 0x00, command list base address, 1K-byte aligned
        uint32_t clbu; // 0x04, command list base address upper 32 bits
        uint32_t fb; // 0x08, FIS base address, 256-byte aligned
        uint32_t fbu; // 0x0C, FIS base address upper 32 bits
        uint32_t is; // 0x10, interrupt status
        uint32_t ie; // 0x14, interrupt enable
        uint32_t cmd; // 0x18, command and status
        uint32_t rsv0; // 0x1C, Reserved
        uint32_t tfd; // 0x20, task file data
        uint32_t sig; // 0x24, signature
        uint32_t ssts; // 0x28, SATA status (SCR0:SStatus)
        uint32_t sctl; // 0x2C, SATA control (SCR2:SControl)
        uint32_t serr; // 0x30, SATA error (SCR1:SError)
        uint32_t sact; // 0x34, SATA active (SCR3:SActive)
        uint32_t ci; // 0x38, command issue
        uint32_t sntf; // 0x3C, SATA notification (SCR4:SNotification)
        uint32_t fbs; // 0x40, FIS-based switch control
        uint32_t rsv1[11]; // 0x44 ~ 0x6F, Reserved
        uint32_t vendor[4]; // 0x70 ~ 0x7F, vendor specific
    };

    class AhciDevice {
    private:
        volatile hba_port_t *port;

    public:
        AhciDevice(volatile hba_port_t *port) : port(port) {

        }

        void configure() {
            printk("Configured!\n");
        }
    };

    class AhciDriver : public Driver {
    private:
        unique_ptr<AhciDevice> devices[32] = {};

        struct hba_t {
            // 0x00 - 0x2B, Generic Host Control
            uint32_t cap; // 0x00, Host capability
            uint32_t ghc; // 0x04, Global host control
            uint32_t is; // 0x08, Interrupt status
            uint32_t pi; // 0x0C, Port implemented
            uint32_t vs; // 0x10, Version
            uint32_t ccc_ctl; // 0x14, Command completion coalescing control
            uint32_t ccc_pts; // 0x18, Command completion coalescing ports
            uint32_t em_loc; // 0x1C, Enclosure management location
            uint32_t em_ctl; // 0x20, Enclosure management control
            uint32_t cap2; // 0x24, Host capabilities extended
            uint32_t bohc; // 0x28, BIOS/OS handoff control and status

            // 0x2C - 0x9F, Reserved
            uint8_t rsv[0xA0-0x2C];

            // 0xA0 - 0xFF, Vendor specific registers
            uint8_t vendor[0x100-0xA0];

            // 0x100 - 0x10FF, Port control registers
            hba_port_t ports[32]; // 1 ~ 32
        };

        volatile hba_t *hba;

    public:
        AhciDriver(Device& device) : Driver(device) {};

        virtual void configure() override {
            // TODO: What happens if the hba crosses over page boundries
            addr_phys_t bar5 = (addr_phys_t)device.get32(0, BAR5);
            addr_phys_t bar5_offset = bar5 % PAGE_SIZE;
            addr_phys_t bar5_base = bar5 - bar5_offset;
            page::Page *page = page::create(bar5_base, page::FLAG_KERNEL, 1);
            hba = (volatile hba_t *)(
                (addr_logical_t)page::kinstall(page, page::PAGE_TABLE_CACHEDISABLE | page::PAGE_TABLE_RW) + bar5_offset);

            // Identify all the devices
            for(int i = 0; i < 32; i ++) {
                if ((hba->pi >> i) & 0x1) {
                    devices[i] = make_unique<AhciDevice>(&(hba->ports[i]));
                    devices[i]->configure();
                }
            }
        }

        virtual void handle_interrupt() override {

        }

        virtual Utf8 device_name() override {
            return Utf8("Unknown Device");
        }

        virtual Utf8 driver_name() override {
            return Utf8("AhciDriver");
        }

        virtual ~AhciDriver() {};
    };

    class AhciDriverFactory : public DriverFactory {
    public:
        virtual void create_driver(Device &device) override {
            device.driver = make_unique<AhciDriver>(device);
        }

        virtual void search_for_device(const vector<Device> &devices) override {
            for(Device &d : devices) {
                if(d.device_class == AHCI_CLASS && d.device_subclass == AHCI_SUBCLASS) {
                    create_driver(d);
                }
            }
        }
    };

    RegisterDriverFactory<AhciDriverFactory> df;
}

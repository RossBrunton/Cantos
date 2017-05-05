#include <stdint.h>

#include "hw/ps2.hpp"
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

namespace ps2 {
    static bool dual = false;
    static bool known_dual = false;
    Ps2Port ports[2];
    static mutex::Mutex _mutex;

    static uint8_t _read_data() {
        return inb(IO_PORT_PS2_DATA);
    }

    static void _write_data(uint8_t dat) {
        outb(IO_PORT_PS2_DATA, dat);
    }

    static void _write_command(uint8_t com) {
        outb(IO_PORT_PS2_COMMAND, com);
    }

    static uint8_t _read_status() {
        return inb(IO_PORT_PS2_STATUS);
    }

    static void _write_double_command(uint8_t com, uint8_t arg) {
        _write_command(com);
        while(_read_status() & STAT_INBUFF) {
            io_wait();
        }
        _write_data(arg);
    }

    static uint8_t _wait_read_data() {
        while(_read_status() & STAT_INBUFF) {
            io_wait();
        }

        return _read_data();
    }

    void _handle1(idt_proc_state_t state) {
        ports[0].handle(state);
    }

    void _handle2(idt_proc_state_t state) {
        ports[1].handle(state);
    }

    void init() {
        uint8_t config;
        uint8_t result;
        // TODO: Check it exists

        _mutex.lock();
        ports[0].second = false;
        ports[1].second = true;

        // Disable devices
        _write_command(COM_DIS1);
        _write_command(COM_DIS2);

        // Discard any data in the buffer
        _read_data();

        // Set initial config
        _write_command(COM_READ);
        config = _wait_read_data();
        if(!(config & CON_CLOCK2)) {
            dual = false;
            known_dual = true;
        }
        config &= ~(CON_EN1 | CON_EN2 | CON_TRANS);
        _write_double_command(COM_WRITE, config);

        // Perform a self test
        _write_command(COM_TESTCON);
        result = _wait_read_data();
        if(result != 0x55) {
            panic("PS/2 failed self-test, got %x", result);
        }

        // Figure out if this is a dual channel port
        if(!known_dual) {
            _write_command(COM_EN2);
            _write_command(COM_READ);
            config = _wait_read_data();
            if(!(config & CON_CLOCK2)) {
                dual = false;
            }else{
                dual = true;
                _write_command(COM_DIS2);
            }
            known_dual = true;
        }

        // Perform tests
        _write_command(COM_TEST1);
        result = _wait_read_data();
        if(!result) {
            ports[0].enabled = true;
            ports[0].init();
        }else{
            kwarn("PS/2 port 0 failed self-test, got %x\n", result);
        }

        _write_command(COM_TEST2);
        result = _wait_read_data();
        if(!result) {
            ports[1].enabled = true;
            ports[1].init();
        }else{
            kwarn("PS/2 port 1 failed self-test, got %x\n", result);
        }

        // Set up interrupts
        ioapic::enable_func(INT_IRQ_MOUSE, _handle2, 0);
        ioapic::enable_func(INT_IRQ_KEYBOARD, _handle1, 0);

        _mutex.unlock();


        printk("Status: %x, Config: %x\n", _read_status(), config);
    }



    void Ps2Port::init() {
        uint8_t info[] = {0, 0};
        uint8_t info_length = 0;
        uint8_t config;

        // Enable it
        if(this->second) {
            _write_command(COM_EN2);
        }else{
            _write_command(COM_EN1);
        }

        // Disable scanning
        this->_write_ack(DEV_DISABLE_SCAN);
        if(this->timed_out) {
            kwarn("PS2 timed out during init (disable scan)\n");
            this->enabled = false;
            return;
        }

        // Send identify command
        this->_write_ack(DEV_IDENTIFY);
        if(this->timed_out) {
            kwarn("PS2 timed out during init (identify)\n");
            this->enabled = false;
            return;
        }

        // Read the results into the info array
        info[0] = this->_read(0xffff);
        if(!this->timed_out) {
            info_length = 1;
            info[1] = this->_read(0xffff);
            if(!this->timed_out) {
                info_length = 2;
            }
        }

        // Then work out what we have
        if(info_length == 0) {
            this->type = TYPE_TRANSLATION_KEYBOARD_AT;
        }else{
            // For some reason, at least under qemu, single codes get duplicated
            switch(info[0]) {
                case 0x00:
                    this->type = TYPE_STANDARD_MOUSE;
                    break;
                case 0x03:
                    this->type = TYPE_SCROLL_MOUSE;
                    break;
                case 0x04:
                    this->type = TYPE_5BTN_MOUSE;
                    break;
                case 0xab:
                    switch(info[1]) {
                        case 0x41:
                        case 0xc1:
                            this->type = TYPE_TRANSLATION_MF2_KEYBOARD;
                            break;
                        case 0x83:
                            this->type = TYPE_MF2_KEYBOARD;
                            break;
                        default:
                            this->type = TYPE_UNKNOWN;
                    }
                    break;
                default:
                    this->type = TYPE_UNKNOWN;
            }
        }

        if(this->type == TYPE_UNKNOWN) {
            kwarn("PS2 device %d is of an unkown type (%x, %x).\n", this->second, info[0], info[1]);
            this->enabled = false;
            return;
        }

        // Now enable it
        _write_command(COM_READ);
        config = _wait_read_data();

        if(this->second) {
            config |= CON_EN2;
        }else{
            config |= CON_EN1;
        }
        _write_double_command(COM_WRITE, config);

        printk("Got information about PS2 port %d - [%x, %x]%d\n", this->second, info[0], info[1], info_length);
    }

    uint8_t Ps2Port::_read(uint32_t timeout) {
        this->timed_out = false;
        while(_read_status() & STAT_INBUFF) {
            io_wait();
            timeout --;
            if(!timeout) {
                this->timed_out = true;
                return 0xff;
            }
        }

        return _read_data();
    }

    void Ps2Port::_write(uint8_t dat) {
        if(this->second) {
            _write_command(COM_NEXT2IN);
        }
        _write_data(dat);
    }

    void Ps2Port::_write_ack(uint8_t dat) {
        uint8_t read = 0;

        do {
            this->_write(dat);

            read = this->_read(0xffff);

            if(read == ACK || this->timed_out) {
                return;
            }
        } while(read == RESEND);
    }

    void Ps2Port::handle(idt_proc_state_t state) {
        printk("Got interrupt!\n");
        lapic::eoi();
    }
}

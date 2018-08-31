#include <stdint.h>

#include "hw/ps2keyboard.hpp"
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

// Silly mappings:
// F7 -> 0x02
// Break -> 0x0f

namespace ps2keyboard {
    uint8_t Ps2KeyboardDriver::send(uint8_t byte) {
        io_wait();
        last_input = -1;

        do {
            port.write(byte);
            while(last_input == -1) {}
        } while(last_input == RESEND);

        return last_input;
    }

    uint8_t Ps2KeyboardDriver::send(uint8_t byte_a, uint8_t byte_b) {
        io_wait();
        last_input = -1;

        do {
            port.write(byte_a);
            port.write(byte_b);
            while(last_input == -1) {}
        } while(last_input == RESEND);

        return last_input;
    }

    void Ps2KeyboardDriver::configure() {
#if DEBUG_PS2
        printk("Configuring a PS2 keyboard on port %d...\n", port.second ? 1 : 0);
#endif
        key = 0;
        double_code = 0;
        last_key = 0;
        self_test_passed = false;

        // Self test
        // TODO: Result
        send(0xff);

        while (!self_test_passed) {}

        // Set to scancode 2
        send(0xf0);
        send(0x02);
    }

    void Ps2KeyboardDriver::handle() {
        uint8_t input = port.read(0xffff);
        if(input == PASS) {
            // Passed a self test
            self_test_passed = true;
            return;
        }

        if(last_input == -1) {
            last_input = (int16_t)input;
            return;
        }

        if(input == ERR_1 || input == ERR_2) {
            // Key detection error, ignore
            return;
        }

        if(input == ECHO || input == ACK || input == RESEND) {
            // Misc stuff, should be handled earlier
            return;
        }

        if(input == ST_FAIL_1 || input == ST_FAIL_2) {
            // Self test fail
            // TODO: Something here
            return;
        }

        if(input == 0x83) {
            // F7 maybe probably generates this key, convent it into 0x02 (which seems to be unused, and is F7 on Linux)
            input = 0x02;
        }

        // Extended keycodes
        if(input == 0xe0) {
            key |= MASK_EXTENDED;
            return;
        }

        // "double" extended keycodes (only pause/break at the moment
        if(input == 0xe1) {
            double_code = true;
            return;
        }

        // Key release code
        if(input == 0xf0) {
            key |= MASK_RELEASE;
            return;
        }

        // Anything else is a literal key code
        if(input < 0x80) {
            if(double_code) {
                // This is the first part of an e1 double code
                last_key = input;
                double_code = false;
                return;
            }else if(last_key) {
                // And the second part of one
                if(last_key == 0x14 && input == 0x77) {
                    // pause/break
                    input = 0x0f;
                }else{
                    kwarn("Unknown e1 scancode %x %x\n", last_key, input);
                    key = 0;
                    last_key = 0;
                    return;
                }
            }

            key |= input;

            printk("0x%x ", key);
            key = 0;
            last_key = 0;
            return;
        }

        // And if we can't recognize it at all, throw a warning
        kwarn("Unknown keyboard scancode %x\n", input);
    }
}

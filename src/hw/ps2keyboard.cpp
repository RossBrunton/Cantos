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
    void Ps2KeyboardDriver::configure() {
        key = 0;
        double_code = 0;
        last_key = 0;
    }

    void Ps2KeyboardDriver::handle() {
        uint8_t input = port.read(0xffff);

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

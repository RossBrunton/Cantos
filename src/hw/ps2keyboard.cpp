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
    void Ps2KeyboardDriver::configure(ps2::Ps2Port *port) {
        this->port = port;
        this->key = 0;
        this->double_code = 0;
        this->last_key = 0;
    }

    void Ps2KeyboardDriver::handle() {
        uint8_t input = this->port->read(0xffff);

        if(input == 0x83) {
            // F7 maybe probably generates this key, convent it into 0x02 (which seems to be unused, and is F7 on Linux)
            input = 0x02;
        }

        // Extended keycodes
        if(input == 0xe0) {
            this->key |= MASK_EXTENDED;
            return;
        }

        // "double" extended keycodes (only pause/break at the moment
        if(input == 0xe1) {
            this->double_code = true;
            return;
        }

        // Key release code
        if(input == 0xf0) {
            this->key |= MASK_RELEASE;
            return;
        }

        // Anything else is a literal key code
        if(input < 0x80) {
            if(this->double_code) {
                // This is the first part of an e1 double code
                this->last_key = input;
                this->double_code = false;
                return;
            }else if(this->last_key) {
                // And the second part of one
                if(this->last_key == 0x14 && input == 0x77) {
                    // pause/break
                    input = 0x0f;
                }else{
                    kwarn("Unknown e1 scancode %x %x\n", this->last_key, input);
                    this->key = 0;
                    this->last_key = 0;
                    return;
                }
            }

            this->key |= input;

            printk("0x%x ", this->key);
            this->key = 0;
            this->last_key = 0;
            return;
        }

        // And if we can't recognize it at all, throw a warning
        kwarn("Unknown keyboard scancode %x\n", input);
    }
}

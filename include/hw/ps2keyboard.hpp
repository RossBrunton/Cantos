#ifndef _HPP_HW_PS2KEYBOARD_
#define _HPP_HW_PS2KEYBOARD_

#include "main/common.h"
#include "hw/ps2.hpp"

namespace ps2keyboard {
    const uint16_t MASK_RELEASE = 0x8000;
    const uint16_t MASK_EXTENDED = 0x0080;

    class Ps2KeyboardDriver : public ps2::Ps2Driver {
    public:
        void configure(ps2::Ps2Port *port);
        void handle();

    private:
        ps2::Ps2Port *port;
        uint16_t key;
        bool double_code;
        uint16_t last_key;
        uint16_t modifiers;
    };
}

#endif

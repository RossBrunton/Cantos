#ifndef _HPP_HW_PS2KEYBOARD_
#define _HPP_HW_PS2KEYBOARD_

#include "main/common.hpp"
#include "hw/ps2.hpp"

namespace ps2keyboard {
    const uint16_t MASK_RELEASE = 0x8000;
    const uint16_t MASK_EXTENDED = 0x0080;

    class Ps2KeyboardDriver : public ps2::Ps2Driver {
    public:
        Ps2KeyboardDriver(ps2::Ps2Port &port) : ps2::Ps2Driver(port) {};

        void configure();
        void handle();

    private:
        uint16_t key;
        bool double_code;
        uint16_t last_key;
        uint16_t modifiers;
    };
}

#endif

#ifndef _HPP_HW_PS2KEYBOARD_
#define _HPP_HW_PS2KEYBOARD_

#include "main/common.hpp"
#include "hw/ps2.hpp"

namespace ps2keyboard {
    const uint16_t MASK_RELEASE = 0x8000;
    const uint16_t MASK_EXTENDED = 0x0080;

    const uint8_t ERR_1 = 0x00;
    const uint8_t PASS = 0xaa;
    const uint8_t ECHO = 0xee;
    const uint8_t ACK = 0xfa;
    const uint8_t ST_FAIL_1 = 0xfc;
    const uint8_t ST_FAIL_2 = 0xfd;
    const uint8_t RESEND = 0xfe;
    const uint8_t ERR_2 = 0xff;

    class Ps2KeyboardDriver : public ps2::Ps2Driver {
    public:
        Ps2KeyboardDriver(ps2::Ps2Port &port) : ps2::Ps2Driver(port) {};

        void configure() override;
        void handle() override;

        uint8_t send(uint8_t byte);
        uint8_t send(uint8_t byte_a, uint8_t byte_b);

    private:
        uint16_t key;
        bool double_code;
        uint16_t last_key;
        uint16_t modifiers;

        int16_t last_input;
    };
}

#endif

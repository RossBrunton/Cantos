#ifndef _HPP_HW_PS2_
#define _HPP_HW_PS2_

#include "main/common.hpp"
#include "structures/unique_ptr.hpp"

namespace ps2 {
    const uint8_t STAT_OUTBUFF = 0x01;
    const uint8_t STAT_INBUFF = 0x02;
    const uint8_t STAT_SYS = 0x04;
    const uint8_t STAT_COMDAT = 0x08;
    const uint8_t STAT_TIMEOUT = 0x40;
    const uint8_t STAT_PARITY = 0x80;

    const uint8_t COM_READ = 0x20;
    const uint8_t COM_WRITE = 0x60;
    const uint8_t COM_DIS2 = 0xa7;
    const uint8_t COM_EN2 = 0xa8;
    const uint8_t COM_TEST2 = 0xa9;
    const uint8_t COM_TESTCON = 0xaa;
    const uint8_t COM_TEST1 = 0xab;
    const uint8_t COM_DIS1 = 0xad;
    const uint8_t COM_EN1 = 0xae;
    const uint8_t COM_NEXT2IN = 0xd4;

    const uint8_t CON_EN1 = 0x01;
    const uint8_t CON_EN2 = 0x02;
    const uint8_t CON_SYS = 0x04;
    const uint8_t CON_CLOCK1 = 0x10;
    const uint8_t CON_CLOCK2 = 0x20;
    const uint8_t CON_TRANS = 0x40;

    const uint8_t DEV_IDENTIFY = 0xf2;
    const uint8_t DEV_DISABLE_SCAN = 0xf5;

    const uint8_t ACK = 0xfa;
    const uint8_t RESEND = 0xfe;

    const uint8_t TYPE_MOUSE = 0x40;
    const uint8_t TYPE_KEYBOARD = 0x80;
    const uint8_t TYPE_TRANSLATION_KEYBOARD_AT = TYPE_KEYBOARD | 0x00;
    const uint8_t TYPE_STANDARD_MOUSE = TYPE_MOUSE | 0x00;
    const uint8_t TYPE_SCROLL_MOUSE = TYPE_MOUSE | 0x01;
    const uint8_t TYPE_5BTN_MOUSE = TYPE_MOUSE | 0x02;
    const uint8_t TYPE_TRANSLATION_MF2_KEYBOARD = TYPE_KEYBOARD | 0x01;
    const uint8_t TYPE_MF2_KEYBOARD = TYPE_KEYBOARD | 0x02;
    const uint8_t TYPE_UNKNOWN = 0x0;

    class Ps2Driver;

    class Ps2Port {
    public:
        bool enabled;
        bool second;
        bool timed_out;
        uint8_t type;
        unique_ptr<Ps2Driver> driver;

        void init();
        void handle(idt_proc_state_t state);

        uint8_t read(uint32_t timeout);
        void write(uint8_t dat);
        void write_ack(uint8_t dat);
    };

    class Ps2Driver {
    protected:
        Ps2Port &port;

    public:
        Ps2Driver(Ps2Port &port) : port(port) {};
        virtual void configure() = 0;
        virtual void handle() = 0;

        virtual ~Ps2Driver() {};
    };

    extern Ps2Port ports[2];

    void init();
}

#endif

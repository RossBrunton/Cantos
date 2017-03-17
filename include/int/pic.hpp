#ifndef _HPP_INT_PIC_
#define _HPP_INT_PIC_

namespace pic {
    const uint8_t INT_MBASE = 0x20;
    const uint8_t INT_SBASE = (INT_MBASE + 8);

    const uint8_t ICW1_ICW4 = 0x01;
    const uint8_t ICW1_SINGLE = 0x02;
    const uint8_t ICW1_INTERVAL4 = 0x04;
    const uint8_t ICW1_LEVEL = 0x08;
    const uint8_t ICW1_INIT = 0x10;

    const uint8_t ICW4_8086 = 0x01;
    const uint8_t ICW4_AUTO = 0x02;
    const uint8_t ICW4_BUF_SLAVE = 0x08;
    const uint8_t ICW4_BUF_MASTER = 0x0C;
    const uint8_t ICW4_SFNM = 0x10;

    void init();
    void enable(uint8_t irq);
    void disable(uint8_t irq);
}

#endif

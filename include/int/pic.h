#ifndef _H_IO_PIC_
#define _H_IO_PIC_

#define PIC_INT_MBASE 0x20
#define PIC_INT_SBASE (PIC_INT_MBASE + 8)

#define PIC_ICW1_ICW4 0x01
#define PIC_ICW1_SINGLE 0x02
#define PIC_ICW1_INTERVAL4 0x04
#define PIC_ICW1_LEVEL 0x08
#define PIC_ICW1_INIT 0x10

#define PIC_ICW4_8086 0x01
#define PIC_ICW4_AUTO 0x02
#define PIC_ICW4_BUF_SLAVE 0x08
#define PIC_ICW4_BUF_MASTER 0x0C
#define PIC_ICW4_SFNM 0x10

void pic_init();
void pic_enable(uint8_t irq);
void pic_disable(uint8_t irq);

#endif

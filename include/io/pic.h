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

#define IRQ_INTERRUPT_TIMER 0x0
#define IRQ_KEYBOARD 0x1
#define IRQ_CASCADE 0x2
#define IRQ_COM2 0x3
#define IRQ_COM1 0x4
#define IRQ_LPT2 0x5
#define IRQ_FLOPPY 0x6
#define IRQ_LPT1 0x7 /* Also "unreliable "spurious" interrupt" */
#define IRQ_CMOS 0x8
#define IRQ_9 0x9
#define IRQ_10 0xa
#define IRQ_11 0xb
#define IRQ_MOUSE 0xc
#define IRQ_FPU 0xd
#define IRQ_PATA 0xe
#define IRQ_SATA 0xf

void pic_init();
void pic_enable(uint8_t irq);
void pic_disable(uint8_t irq);

#endif

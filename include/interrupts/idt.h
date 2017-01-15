#ifndef _H_INTERRUPTS_IDT_
#define _H_INTERRUPTS_IDT_

/** @file interrupts/idt.h
 *
 * Handles setting up the IDT, and managing interrupts.
 *
 * Also includes a structure representing the state of the CPU (as used by interrupts) and functions for adding
 *  vaules to the IDT.
 *
 * The IDT itself will be created and added by calling @ref idt_init, and is stored statically. New entries may be added
 *  to this table using @ref idt_install.
 *
 * The difference between a trap gate and an interrupt gate, is that the interrupt gate disables interrupts while it is
 *  running.
 * 
 * @sa http://wiki.osdev.org/IDT
 */

#include <stdint.h>

/** An IDT descriptor, as used by the `LIDT` instruction.
 */
typedef struct __attribute__((packed)) idt_descriptor_s {
    uint16_t size; /**< The size of the IDT table in bytes, minus one */
    uint32_t offset; /**< The location of the IDT table in memory */
} idt_descriptor_t;

/** An entry in the IDT */
typedef struct idt_entry_s {
    uint16_t offset_low; /**< Bits 0..15 of the location of the handler */
    uint16_t selector; /**< Segment selector */
    uint8_t zero; /**< Must be zero */
    uint8_t type_attr; /**< A set of flags followed by a gate type */
    uint16_t offset_high; /**< Bits 16..31 of the location of the handler */
} idt_entry_t;

/** A type attr flag that if set, means the IDT entry is present */
#define IDT_FLAG_PRESENT (1 << 7)
/** A type attr flag macro which sets which privilige level the calling discriptor must have */
#define IDT_FLAG_DPL(x) ((x) << 5)

/** 32 bit task gate */
#define IDT_GATE_32_TASK 0x5
/** 16 bit interrupt gate */
#define IDT_GATE_16_INT 0x6
/** 16 bit trap gate */
#define IDT_GATE_16_TRAP 0x7
/** 32 bit interrupt gate */
#define IDT_GATE_32_INT 0xe
/** 32 bit trap gate */
#define IDT_GATE_32_TRAP 0xf

/** Sets the appropriate values on the given entry.
 *
 * @param[in,out] entry The entry to set the values for, must be a valid pointer.
 * @param[in] offset The address of the function to jump to.
 * @param[in] selector The segment selector to use.
 * @param[in] type_attr The value to set as the type attr.
 */
void idt_set_entry(idt_entry_t *entry, uint32_t offset, uint16_t selector, uint8_t type_attr);
/** Installs a handler into the system IDT.
 *
 * @ref idt_init must have been called first.
 *
 * @param[in] id The ID of the interrupt to register for.
 * @param[in] offset The address of the function to jump to.
 * @param[in] selector The segment selector to use.
 * @param[in] type_attr The value to set as the type attr.
 */
void idt_install(uint8_t id, uint32_t offset, uint16_t selector, uint8_t type_attr);
/** Sets up and enables the system IDT.
 *
 * The created IDT will be enabled and loaded into the CPU (using LIDT), and all of its entries will be marked as "not
 *  present".
 */
void idt_init();

#endif

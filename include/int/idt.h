#ifndef _H_INTERRUPTS_IDT_
#define _H_INTERRUPTS_IDT_

/** @file int/idt.h
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

#include "main/common.h"

typedef void (* idt_interrupt_handler_t)(idt_proc_state_t);
typedef void (* idt_interrupt_handler_err_t)(idt_proc_state_t, uint32_t);

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

/** Enables the interrupt in general.
 *
 * This adds an entry to the system IDT for that interrupt.
 *
 * @todo Document this more.
 */
#ifdef __cplusplus
#define IDT_TELL_INTERRUPT(name) extern "C" void idt_asm_interrupt_ ## name ()
#define IDT_ALLOW_INTERRUPT(id, name) do {\
    idt_enable_entry(id, (uint32_t) idt_asm_interrupt_ ## name);\
} while(0)
#else
#define IDT_ALLOW_INTERRUPT(id, name) do {\
    extern void idt_asm_interrupt_ ## name ();\
    idt_enable_entry(id, (uint32_t) idt_asm_interrupt_ ## name);\
} while(0)
#endif

/** Sets the appropriate entry point for the given entry.
 *
 * Typically you should use the ALLOW_INTERRUPT or ALLOW_INTERRUPT_ERR macros to call this.
 *
 * @param[in] vector The vector to enable.
 * @param[in] offset The address of the function to jump to.
 */
void idt_enable_entry(uint8_t vector, uint32_t offset);
/** Sets the appropriate values on the given entry.
 *
 * @param[in,out] entry The entry to set the values for, must be a valid pointer.
 * @param[in] selector The segment selector to use.
 * @param[in] type_attr The value to set as the type attr.
 */
void idt_update_entry(idt_entry_t *entry, uint16_t selector, uint8_t type_attr);
/** Installs a handler into the system IDT.
 *
 * @ref idt_init must have been called first.
 *
 * @param[in] id The ID of the interrupt to register for.
 * @param[in] offset The address of the function to jump to.
 * @param[in] selector The segment selector to use.
 * @param[in] type_attr The value to set as the type attr.
 */
void idt_install(uint8_t id, idt_interrupt_handler_t offset, uint16_t selector, uint8_t type_attr);

void idt_install_with_error(uint8_t id, idt_interrupt_handler_err_t offset, uint16_t selector, uint8_t type_attr);
/** Sets up and enables the system IDT.
 *
 * The created IDT will be enabled and loaded into the CPU (using LIDT), and all of its entries will be marked as "not
 *  present".
 */
void idt_init();

void idt_setup();

void idt_handle(uint32_t vector, idt_proc_state_t state);
void idt_handle_with_error(uint32_t vector, idt_proc_state_t state, uint32_t errcode);

void idt_asm_handle();
void idt_asm_handle_err();

#endif

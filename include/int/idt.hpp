#ifndef _HPP_INTERRUPTS_IDT_
#define _HPP_INTERRUPTS_IDT_

/** @file int/idt.h
 *
 * Handles setting up the IDT, and managing interrupts.
 *
 * Also includes a structure representing the state of the CPU (as used by interrupts) and functions for adding
 *  vaules to the IDT.
 *
 * The IDT itself will be created and added by calling @ref init, and is stored statically. New entries may be added
 *  to this table using @ref install.
 *
 * The difference between a trap gate and an interrupt gate, is that the interrupt gate disables interrupts while it is
 *  running.
 * 
 * @sa http://wiki.osdev.org/IDT
 */

#include <stdint.h>

#include "main/common.h"

namespace idt {
    typedef void (* interrupt_handler_t)(idt_proc_state_t);
    typedef void (* interrupt_handler_err_t)(idt_proc_state_t, uint32_t);

    /** An IDT descriptor, as used by the `LIDT` instruction.
     */
    typedef struct __attribute__((packed)) descriptor_s {
        uint16_t size; /**< The size of the IDT table in bytes, minus one */
        uint32_t offset; /**< The location of the IDT table in memory */
    } descriptor_t;

    extern "C" volatile descriptor_t idt_descriptor;

    /** An entry in the IDT */
    typedef struct entry_s {
        uint16_t offset_low; /**< Bits 0..15 of the location of the handler */
        uint16_t selector; /**< Segment selector */
        uint8_t zero; /**< Must be zero */
        uint8_t type_attr; /**< A set of flags followed by a gate type */
        uint16_t offset_high; /**< Bits 16..31 of the location of the handler */
    } entry_t;

    /** A type attr flag that if set, means the IDT entry is present */
    const uint8_t FLAG_PRESENT = (1 << 7);
    /** A type attr flag macro which sets which privilige level the calling discriptor must have */
    #define IDT_FLAG_DPL(x) ((x) << 5)

    /** 32 bit task gate */
    const uint8_t GATE_32_TASK = 0x5;
    /** 16 bit interrupt gate */
    const uint8_t GATE_16_INT = 0x6;
    /** 16 bit trap gate */
    const uint8_t GATE_16_TRAP = 0x7;
    /** 32 bit interrupt gate */
    const uint8_t GATE_32_INT = 0xe;
    /** 32 bit trap gate */
    const uint8_t GATE_32_TRAP = 0xf;

    /** Enables the interrupt in general.
     *
     * This adds an entry to the system IDT for that interrupt.
     *
     * @todo Document this more.
     */
    #define IDT_TELL_INTERRUPT(name) extern "C" void idt_asm_interrupt_ ## name ()
    #define IDT_ALLOW_INTERRUPT(id, name) do {\
        idt::enable_entry(id, (uint32_t) idt_asm_interrupt_ ## name);\
    } while(0)

    /** Sets the appropriate entry point for the given entry.
     *
     * Typically you should use the ALLOW_INTERRUPT or ALLOW_INTERRUPT_ERR macros to call this.
     *
     * @param[in] vector The vector to enable.
     * @param[in] offset The address of the function to jump to.
     */
    void enable_entry(uint8_t vector, uint32_t offset);
    /** Sets the appropriate values on the given entry.
     *
     * @param[in,out] entry The entry to set the values for, must be a valid pointer.
     * @param[in] selector The segment selector to use.
     * @param[in] type_attr The value to set as the type attr.
     */
    void update_entry(entry_t *entry, uint16_t selector, uint8_t type_attr);
    /** Installs a handler into the system IDT.
     *
     * @ref init must have been called first.
     *
     * @param[in] id The ID of the interrupt to register for.
     * @param[in] offset The address of the function to jump to.
     * @param[in] selector The segment selector to use.
     * @param[in] type_attr The value to set as the type attr.
     */
    void install(uint8_t id, interrupt_handler_t offset, uint16_t selector, uint8_t type_attr);

    void install_with_error(uint8_t id, interrupt_handler_err_t offset, uint16_t selector, uint8_t type_attr);
    /** Sets up and enables the system IDT.
     *
     * The created IDT will be enabled and loaded into the CPU (using LIDT), and all of its entries will be marked as "not
     *  present".
     */
    void init();
    void setup();

    void handle(uint32_t vector, idt_proc_state_t state);
    void handle_with_error(uint32_t vector, idt_proc_state_t state, uint32_t errcode);
    
    extern "C" {
        void idt_handle(uint32_t vector, idt_proc_state_t state);
        void idt_handle_with_error(uint32_t vector, idt_proc_state_t state, uint32_t errcode);
    }

    void asm_handle();
    void asm_handle_err();
}

#endif

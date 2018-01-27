#ifndef _H_MAIN_COMMON_
#define _H_MAIN_COMMON_

/** @file main/common.hpp
 *
 * Contains common definitions used by multiple files and which aren't specific to any one.
 *
 * Including common.hpp will also include the following headers (which should be seen as "always available"):
 *
 * * stdint.h
 * * asm_utils.hpp
 * * panic.hpp
 * * printk.hpp
 * * list.hpp
 * * mutex.hpp
 * * shared_ptr.hpp
 * * unique_ptr.hpp
 * * utf8.hpp
 * * errno.h
 * * failable.hpp
 *
 * In addition, the following will be made available in the global scope, and thus won't require namespace scoping:
 *
 * * list_ns::list
 * * mutex::Mutex
 * * shared_ptr_ns::shared_ptr
 * * shared_ptr_ns::make_shared
 * * unique_ptr_ns::unique_ptr
 * * unique_ptr_ns::make_unique
 * * utf8::Utf8
 * * failable::Failable
 */

#include <stdint.h>

#include "structures/shared_ptr.hpp"
using shared_ptr_ns::shared_ptr;
using shared_ptr_ns::make_shared;
#include "structures/unique_ptr.hpp"
using unique_ptr_ns::unique_ptr;
using unique_ptr_ns::make_unique;
#include "main/asm_utils.hpp"
#include "main/panic.hpp"
#include "main/printk.hpp"
#include "structures/list.hpp"
#include "structures/mutex.hpp"
#include "structures/shared_ptr.hpp"
#include "structures/unique_ptr.hpp"
#include "structures/utf8.hpp"
#include "main/errno.h"
#include "structures/failable.hpp"

using list_ns::list;
using mutex::Mutex;
using utf8::Utf8;
using failable::Failable;

/** Processor state, in the same format as pushed by the `PUSHAD` instruction.
 *
 * That is, `PUSHAD` before a function call will have a structure of this type as its argument.
 */
typedef struct idt_proc_state_s {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
} idt_proc_state_t;

/** Variables of this type represent a physical address
 *
 * That is, an address in physical memory, ignoring the page table.
 *
 * This can be cast to and from a void * freely.
 */
typedef uintptr_t addr_phys_t;

/** Variables of this type represent a logical address
 *
 * That is, an address after paging has been applied.
 *
 * This can be cast to and from a void * freely.
 */
typedef uintptr_t addr_logical_t;

#define PAGE_SIZE 0x1000
#define PAGE_DIR_SIZE 0x400000
#define PAGE_TABLE_LENGTH 0x400

#define TOTAL_VM_SIZE 0x100000000
#define KERNEL_VM_SIZE 0x40000000
#define KERNEL_VM_PAGES (KERNEL_VM_SIZE / PAGE_SIZE)
#define KERNEL_VM_PAGE_TABLES (KERNEL_VM_PAGES / PAGE_TABLE_LENGTH)
#define KERNEL_VM_BASE (TOTAL_VM_SIZE - KERNEL_VM_SIZE) // Higher half kernel


// Cast to the low address of a variable
#define LOW(t, x) (*(t *)((addr_phys_t)&x - (addr_phys_t)KERNEL_VM_BASE))

#define MAX_CORES 32

template<class T> using callback_t = void (*)(T);

#if CHECK_IF
#include "main/panic.hpp"
#define CHECK_IF_SET {uint32_t flags; asm ("pushf; pop %0;" : "=r"(flags)); if(!(flags & 0x200)) panic("IF not set in %s", __func__);}
#define CHECK_IF_CLR {uint32_t flags; asm ("pushf; pop %0;" : "=r"(flags)); if(flags & 0x200) panic("IF not clear in %s", __func__);}
#else
#define CHECK_IF_SET
#define CHECK_IF_CLR
#endif

#endif

#ifndef _HPP_MAIN_LOMAIN_
#define _HPP_MAIN_LOMAIN_

/** @file main/lomain.hpp
 *
 * Used as part of the initialization, to handle setting up the kernel located in the higher half of memory.
 *
 * After @ref kmem_init has been called, anything in this namespace will be unavailable.
 *
 * The typical flow is as follows:
 * * Control enters boot.s
 * * `boot.s` creates a stack for C code
 * * `boot.s` calls `low_kernel_main`
 * * `low_kernel_main` sets up the memory map, with the rest of the kernel above `KERNEL_VM_BASE`
 * * `boot.s` applies the returned memory map to the processor
 * * `boot.s` then calls `kernel_main` which then does everything else
 */

extern "C" {
#include "mem/kmem.h"
}

/** Fills in the other structs in this namespace, and sets up a memory map which is returned.
 *
 * The returned memory map will have the first 4MiB identity mapped and the kernel mapped above `KERNEL_VM_BASE` where
 *  it expects to be.
 *
 * @param[in] mbi The multiboot header, from the bootloader
 * @return The page dir for a page table to use for the kernel
 */
extern "C" volatile page_dir_t *low_kernel_main(multiboot::info_t *mbi);

#endif

#ifndef _HPP_MEM_KMEM_
#define _HPP_MEM_KMEM_

#include <stdint.h>
#include <stddef.h>

#include "mem/page.hpp"
#include "main/common.h"

/** Manages kernel memory allocation
  *
  * This mainly provides two functions, kmem::kmalloc and kmem::kfree, which function similarly to `malloc` and `free`.
  *  These functions are also called internally when a C++ object is allocated or destroyed (using `new` or `delete`).
  *
  * Memory allocated through this class is located in kernel memory, meaning it is visible to all threads running in
  *  kernel mode.
  *
  * The memory pool used by kmem grows automatically when it is full.
  *
  * kmem::kmalloc and kmem::kfree are available in the global scope as `kmalloc` and `kfree`.
  *
  * @TODO This is not thread safe!
  */
namespace kmem {
    /** A kmalloc flag that if set indicates that the reserved area should be used
     *
     * A small amount of memory is reserved for allocations with this flag. This prevents growth of the memory pool,
     *  which in turn prevents additional memory allocations for the data structures required.
     *
     * This shouldn't be used unless having a kmalloc as a side effect of your kmalloc is bad. In most cases, you don't
     *  care.
     */
    const uint8_t KMALLOC_RESERVED = (1 << 0);

    /** A kernel memory map, indicating where different areas of the kernel lie. */
    typedef struct map_s {
        addr_logical_t kernel_ro_start; /**< Base address of kernel readonly memory */
        addr_logical_t kernel_ro_end; /**< End address of kernel readonly memory */
        addr_logical_t kernel_rw_start; /**< Base address of kernel read/write memory */
        addr_logical_t kernel_rw_end; /**< End address of kernel read/write memory */
        addr_logical_t kernel_info_start; /**< Base address of the kernel's ELF information */
        addr_logical_t kernel_info_end; /**< End address of the kernel's ELF information */
        addr_logical_t vm_start; /**< Base address of the kernel's virtual memory tables */
        addr_logical_t vm_end; /**< End address of the kernel's virtual memory tables */
        addr_logical_t memory_start; /**< Base address of the kernel's free use memory */
        addr_logical_t memory_end; /**< End address of the kernel's free use memory (this may grow over time) */
    } map_t;
    /** The memory map of the kernel
     *
     * kmem::init must be called before using this struct's values.
     */
    extern map_t map;

    /** Initialises the kmem system, this must be called before any dynamic memory is used
     *
     * This populates kmem::map, calls page::init and sets up kmem for memory allocation.
     */
    void init();
    /** Allocate `size` bytes of kernel memory, and return a pointer to the start of the allocated memory
     *
     * If for some reason this is impossible (e.g. running out of virtual/physical memory), we panic instead.
     *
     * Allocating 0 bytes does nothing.
     *
     * @param size The number of bytes to allocate.
     * @param flags Any flags that change the behaviour of the allocation.
     * @return A pointer to the newly allocated memory region, or `nullptr` if size is 0.
     */
    void *__attribute__((alloc_size(1), malloc)) kmalloc(size_t size, uint8_t flags);
    /** Frees previously allocated memory
     *
     * The pointer supplied to this function must be the same pointer received from a previous call to kmem::kmalloc.
     *
     * Attempting to free `nullptr` has no effect.
     *
     * @param ptr A pointer to the memory to free
     */
    void kfree(void *ptr);
    /** Remove the bottom page from the kernel memory map
     *
     * After this call, memory adresses lower than 4MiB will be unavailable.
     */
    void clear_bottom();
}

using kmem::kmalloc;
using kmem::kfree;

#endif

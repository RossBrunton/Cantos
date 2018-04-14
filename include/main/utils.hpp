#pragma once

#include "main/panic.hpp"
#include <stddef.h>

/** @file main/utils.hpp
 *
 * Contains a number of utility functions, stuff like memcpy and strlen
 */

inline void* memcpy(void* destination, const void* source, size_t num) {
    for (size_t i = 0; i < num; i++) {
        ((char*)destination)[i] = ((char*)source)[i];
    }
    return destination;
}

inline size_t strlen(const char* str) {
    if (!str) {
        panic("Null in _strlen");
    }
    size_t p = 0;
    while (p < UINT32_MAX) {
        if (str[p] == '\0') {
            return p;
        }
        p++;
    }
    return UINT32_MAX - 1;
}

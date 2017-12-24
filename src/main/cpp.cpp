#include <stddef.h>

#include "main/panic.hpp"
#include "mem/kmem.hpp"
#include "main/cpp.hpp"

extern "C" void __cxa_pure_virtual() {
    panic((char *)"__cxa_pure_virtual called.");
}

void *operator new(size_t size) {
    return kmalloc(size, 0);
}

void *operator new[](size_t size) {
    return kmalloc(size, 0);
}

void operator delete(void *p) {
    kfree(p);
}

void operator delete[](void *p) {
    kfree(p);
}

void operator delete(void *p, size_t size) {
    kfree(p);
}

void operator delete[](void *p, size_t size) {
    kfree(p);
}

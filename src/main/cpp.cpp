#include <stddef.h>

#include "main/cpp.hpp"
#include "main/common.hpp"

extern "C" void __cxa_pure_virtual() {
    panic((char *)"__cxa_pure_virtual called.");
}

void *operator new(size_t size) {
    return kmem::kmalloc(size, 0);
}

void *operator new[](size_t size) {
    return kmem::kmalloc(size, 0);
}

void *operator new(size_t size, void *pos) {
    return pos;
}

void *operator new[](size_t size, void *pos) {
    return pos;
}

void operator delete(void *p) {
    kmem::kfree(p);
}

void operator delete[](void *p) {
    kmem::kfree(p);
}

void operator delete(void *p, size_t size) {
    kmem::kfree(p);
}

void operator delete[](void *p, size_t size) {
    kmem::kfree(p);
}

// Destructors (Do nothing, because we aren't exiting ever
extern "C" int __cxa_atexit(void (*destructor) (void *), void *arg, void *dso) {return 0;}
extern "C" void __cxa_finalize(void *f) {}

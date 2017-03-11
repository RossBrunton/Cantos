#ifndef _HPP_TASK_TASK_
#define _HPP_TASK_TASK_

#include <stddef.h>

#include "mem/vm.hpp"
#include "mem/object.hpp"

extern "C" {
    #include "mem/page.h"
}

namespace task {
    const uint32_t TASK_STACK_TOP = KERNEL_VM_BASE;

    class Process;
    class Thread;

    class Process {
    public:
        Thread *thread;
        uint32_t process_id;
        uint32_t owner;
        uint32_t group;
        uint32_t thread_counter;

        Process *next;

        Process(uint32_t owner, uint32_t group);
        Thread* new_thread(addr_logical_t entry_point);
    };

    class Thread {
    public:
        Process *process;

        uint32_t thread_id;
        uint32_t task_id;

        vm::Map *vm;

        object::Object *stack;
        addr_logical_t stack_pointer;

        Thread *next_in_process;
        Thread *next_in_tasks;

        Thread(Process *process, addr_logical_t entry);
        ~Thread();
    };

    extern Process kernel_process;

    void init();

    extern "C" void __attribute__((noreturn)) task_enter(Thread *thread);
    extern "C" void task_yield();
    extern "C" void task_yield_done(uint32_t sp);
    extern "C" void task_timer_yield();
}

#endif

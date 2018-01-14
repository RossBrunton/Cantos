#ifndef _HPP_TASK_TASK_
#define _HPP_TASK_TASK_

#include <stddef.h>

#include "mem/vm.hpp"
#include "mem/object.hpp"
#include "mem/page.hpp"
#include "structures/unique_ptr.hpp"
#include "structures/list.hpp"
#include "structures/shared_ptr.hpp"

namespace task {
    const uint32_t TASK_STACK_TOP = KERNEL_VM_BASE;

    class Process;
    class Thread;

    class Process {
    private:
        list<shared_ptr<Thread>> threads;

    public:
        uint32_t process_id;
        uint32_t owner;
        uint32_t group;
        uint32_t thread_counter;

        Process(uint32_t owner, uint32_t group);
        shared_ptr<Thread> new_thread(addr_logical_t entry_point);
        shared_ptr<Thread> get_thread(uint32_t id) const;
        void remove_thread(uint32_t id);
    };

    class Thread {
    public:
        shared_ptr<Process> process;

        uint32_t thread_id;
        uint32_t task_id;

        unique_ptr<vm::Map> vm;

        shared_ptr<object::Object> stack;
        addr_logical_t stack_pointer;

        bool in_use;
        bool ended;

        Thread(shared_ptr<Process> process, addr_logical_t entry);
        ~Thread();

        void end();
    };

    extern shared_ptr<Process> kernel_process;
    shared_ptr<Process> get_process(uint32_t id);

    void init();

    extern "C" void __attribute__((noreturn)) task_enter(shared_ptr<Thread> thread);
    extern "C" void task_yield();
    extern "C" void __attribute__((noreturn)) task_yield_done(uint32_t sp);
    extern "C" void task_timer_yield();
    extern "C" void __attribute__((noreturn)) task_end();
    void __attribute__((noreturn)) schedule();


    bool in_thread();
    shared_ptr<Thread> get_thread();
}

#endif

#include <stdint.h>
#include <stdbool.h>

#include "task/task.hpp"
#include "main/panic.hpp"
#include "main/cpu.hpp"
#include "main/printk.hpp"
#include "mem/object.hpp"
#include "mem/kmem.hpp"
#include "structures/mutex.hpp"
#include "structures/shared_ptr.hpp"
#include "structures/list.hpp"
#include "structures/static_list.hpp"
#include "main/asm_utils.hpp"

extern "C" {
    #include "task/asm.h"
}

namespace task {
    const uint8_t _INIT_FLAGS = 0x0;

    Process kernel_process(0, 0);

    static uint32_t thread_counter;
    static uint32_t process_counter;
    static uint32_t task_counter;

    static Thread * volatile tasks;

    static mutex::Mutex waiting_mutex;
    static mutex::Mutex _mutex;

    list<Thread *> waiting_threads;

    static void *_memcpy(void *destination, const void *source, size_t num) {
        size_t i;
        for(i = 0; i < num; i ++) {
            ((char *)destination)[i] = ((char *)source)[i];
        }
        return destination;
    }


    void init() {
        kernel_process.process_id = 0;
        // All other fields 0 by default
    }


    Process::Process(uint32_t owner, uint32_t group) {

    }


    /**
     * @todo Copy all the objects into the new memory map
     * @todo Get the stack object properly
     */
    Thread::Thread(Process *process, addr_logical_t entry)
        : process(process), thread_id(++process->thread_counter), task_id(++task_counter), in_use(false) {
        bool kernel = process->process_id == 0;
        uint32_t *sp;
        idt_proc_state_t pstate = {0, 0, 0, 0, 0, 0, 0, 0};
        void *stack_installed;

        _mutex.lock();

        next_in_process = process->thread;
        process->thread = this;

        // Create the virtual memory map
        vm = make_unique<vm::Map>(process->process_id, task_id, kernel);

        // Create the stack object
        stack = make_shared<object::Object>(object::gen_empty, object::del_free, 1, page::PAGE_TABLE_RW, 0, 0);
        stack->generate(0, 1);

        vm->add_object(stack, TASK_STACK_TOP - PAGE_SIZE, 0, 1);

        stack_installed = page::kinstall(stack->pages->page, 0);

        // Initial stack format:
        // [pushad values]
        // entry eip
        sp = (uint32_t *)((addr_logical_t)stack_installed + PAGE_SIZE) - 1;
        *sp = entry;
        sp --;
        *sp = (addr_logical_t)task_asm_entry_point;
        sp --;
        *sp = _INIT_FLAGS;
        sp -= (sizeof(pstate) / 4);
        _memcpy(sp - (sizeof(pstate) / 4), &pstate, sizeof(pstate));

        stack_pointer = TASK_STACK_TOP - sizeof(void *) * 3 - sizeof(pstate);

        page::kuninstall(stack_installed, stack->pages->page);

        next_in_tasks = (Thread *)tasks;
        tasks = (Thread* volatile)this;

        waiting_threads.push_front(this);
        _mutex.unlock();
    }


    Thread::~Thread() {
        Thread *now;
        Thread *prev = NULL;

        _mutex.lock();

        // Remove it from the process thread list
        for(now = this->process->thread; now != this; (prev = now), (now = now->next_in_process));
        if(prev) {
            prev->next_in_process = this->next_in_process;
        }else{
            this->process->thread = this->next_in_process;
        }

        if(!now) {
            panic("Thread to destroy not found in it's process' threads.");
        }

        // And the main thread list
        prev = NULL;
        for(now = (Thread *)tasks; now != this; (prev = now), (now = now->next_in_tasks));
        if(prev) {
            prev->next_in_tasks = this->next_in_tasks;
        }else{
            tasks = this->next_in_tasks;
        }

        if(!now) {
            panic("Thread to destroy not found in task lists.");
        }

        _mutex.unlock();
    }


    extern "C" void __attribute__((noreturn)) task_enter(Thread *thread) {
        asm volatile ("cli");
        cpu::Status &info = cpu::info();
        info.thread = thread;

        // Load the task's memory map
        vm::table_switch(thread->vm->physical_dir->mem_base);

        // And then hop into it
        task_asm_enter(thread->stack_pointer);
    }

    extern "C" void task_yield() {
        uint32_t eflags = push_cli();
        cpu::Status& info = cpu::info();
        bool in_thread = info.thread != 0;
        uint32_t stack = (uint32_t)info.stack + PAGE_SIZE;
        pop_flags(eflags);

        // Do nothing if we are not in a task
        if(in_thread) {
            return;
        }

        // Call the exit function to move over the stack, will call task_yield_done
        task_asm_yield(stack);
    }

    extern "C" void __attribute__((noreturn)) task_yield_done(uint32_t sp) {
        Thread *current;
        cpu::Status& info = cpu::info();
        current = info.thread;
        info.thread = nullptr;
        current->stack_pointer = sp;

        // And then use the "normal" memory map
        vm::table_clear();

        current->in_use = false;

        // We are now free and can be interrupted again
        asm volatile ("sti");

        waiting_mutex.lock();
        waiting_threads.push_front(current);
        waiting_mutex.unlock();

        schedule();
    }

    void __attribute__((noreturn)) schedule() {
        Thread *next;

        while(true) {
            waiting_mutex.lock();
            if(!waiting_threads.empty()) {
                next = waiting_threads.back();
                waiting_threads.pop_back();
                waiting_mutex.unlock();
                break;
            }
            waiting_mutex.unlock();
            asm volatile ("hlt");
        }

        _mutex.lock();
        bool ok = true;

        if(next->in_use) {
            ok = false;
            panic("Found an in use thread in the waiting list");
        }

        if(ok) {
            next->in_use = true;
        }

        _mutex.unlock();

        if(ok) {
            task_enter(next);
        }else{
            schedule();
        }
    }


    extern "C" void task_timer_yield() {
        cpu::Status& info = cpu::info();

        if(!info.thread) {
            // Not running a thread, do nothing
            return;
        }

        task_yield();
    }

    bool in_thread() {
        bool to_ret = false;

        uint32_t eflags = push_cli();
        cpu::Status& info = cpu::info();
        to_ret = info.thread != 0;
        pop_flags(eflags);

        return to_ret;
    }
}

#include <stdint.h>
#include <stdbool.h>

#include "task/task.hpp"
#include "main/panic.hpp"
#include "main/cpu.hpp"
#include "main/printk.hpp"
#include "mem/object.hpp"
#include "mem/kmem.hpp"
#include "structures/mutex.hpp"

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
    static mutex::Mutex _mutex;

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


    static Thread *_next_task(Thread *task) {
        if(task->next_in_tasks) {
            return task->next_in_tasks;
        }else{
            return (Thread *)tasks;
        }
    }


    Process::Process(uint32_t owner, uint32_t group) {

    }


    /**
     * @todo Copy all the objects into the new memory map
     * @todo Get the stack object properly
     */
    Thread::Thread(Process *process, addr_logical_t entry) {
        bool kernel = process->process_id == 0;
        uint32_t *sp;
        idt_proc_state_t pstate = {0, 0, 0, 0, 0, 0, 0, 0};
        void *stack_installed;

        _mutex.lock();
        this->in_use = false;

        this->next_in_process = process->thread;
        process->thread = this;
        this->process = process;
        this->thread_id = ++process->thread_counter;
        this->task_id = ++task_counter;

        // Create the virtual memory map
        this->vm = new vm::Map(process->process_id, this->task_id, kernel);

        // Create the stack object
        this->stack = new object::Object(object::gen_empty, object::del_free, 1, page::PAGE_TABLE_RW, object::FLAG_AUTOFREE, 0);
        this->stack->generate(0, 1);
        this->stack->add_to_vm(this->vm, TASK_STACK_TOP - PAGE_SIZE);

        stack_installed = page::kinstall(this->stack->pages->page, 0);

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

        this->stack_pointer = TASK_STACK_TOP - sizeof(void *) * 3 - sizeof(pstate);

        page::kuninstall(stack_installed, this->stack->pages->page);

        this->next_in_tasks = (Thread *)tasks;
        tasks = (Thread* volatile)this;

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

        // And delete the memory map
        delete this->vm;
        _mutex.unlock();
    }


    extern "C" void __attribute__((noreturn)) task_enter(Thread *thread) {
        cpu::Status *info;

        info = cpu::info();
        info->thread = thread;

        // Load the task's memory map
        vm::table_switch(thread->vm->physical_dir->mem_base);

        // And then hop into it
        task_asm_enter(thread->stack_pointer);
    }

    extern "C" void task_yield() {
        cpu::Status *info;
        info = cpu::info();

        // Do nothing if we are not in a task
        if(!info->thread) {
            return;
        }

        // Call the exit function to move over the stack, will call task_yield_done
        task_asm_yield((uint32_t)info->stack + PAGE_SIZE);
    }

    extern "C" void __attribute__((noreturn)) task_yield_done(uint32_t sp) {
        cpu::Status *info;
        Thread *current;
        info = cpu::info();
        current = info->thread;
        info->thread = NULL;

        _mutex.lock();

        current->stack_pointer = sp;

        // And then use the "normal" memory map
        vm::table_clear();

        current->in_use = false;

        _mutex.unlock();

        schedule(current);
    }

    void __attribute__((noreturn)) schedule(Thread *base) {
        while(tasks == NULL);

        while(true) {
            Thread *next;
            bool ok = false;

            _mutex.lock();

            if(!base) {
                base = (Thread *)tasks;
            }

            ok = false;
            for(next = _next_task(base); true; next = _next_task(next)) {
                ok = true;

                if(next->in_use) {
                    ok = false;
                }

                if(next == base || ok) {
                    break;
                }
            }

            if(ok) {
                next->in_use = true;
            }

            _mutex.unlock();

            if(ok) {
                task_enter(next);
            }else{
                asm volatile ("hlt");
            }
        }
    }


    extern "C" void task_timer_yield() {
        cpu::Status *info;
        info = cpu::info();

        if(!info->thread) {
            // Not running a thread, do nothing
            return;
        }

        task_yield();
    }
}

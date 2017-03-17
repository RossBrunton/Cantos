#include <stdint.h>
#include <stdbool.h>

#include "task/task.hpp"
#include "main/panic.hpp"
#include "main/cpu.hpp"
#include "main/printk.hpp"
#include "mem/object.hpp"

extern "C" {
    #include "mem/kmem.h"
    #include "mem/gdt.h"
    #include "task/asm.h"
}

namespace task {
    const uint8_t _INIT_FLAGS = 0x0;

    Process kernel_process(0, 0);

    static uint32_t thread_counter;
    static uint32_t process_counter;
    static uint32_t task_counter;

    static Thread *tasks;

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
            return tasks;
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

        this->next_in_process = process->thread;
        process->thread = this;
        this->process = process;
        this->thread_id = ++process->thread_counter;
        this->task_id = ++task_counter;

        // Create the virtual memory map
        this->vm = new vm::Map(process->process_id, this->task_id, kernel);

        // Create the stack object
        this->stack = new object::Object(object::gen_empty, object::del_free, 1, PAGE_TABLE_RW, object::FLAG_AUTOFREE);
        this->stack->generate(0, 1);
        this->stack->add_to_vm(this->vm, TASK_STACK_TOP - PAGE_SIZE);

        stack_installed = page_kinstall(this->stack->pages->page, 0);

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

        page_kuninstall(stack_installed, this->stack->pages->page);

        this->next_in_tasks = tasks;
        tasks = this;
    }


    Thread::~Thread() {
        Thread *now;
        Thread *prev = NULL;

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
        for(now = tasks; now != this; (prev = now), (now = now->next_in_tasks));
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

        // Call the exit function to move over the stack, will call task_yield_done
        task_asm_yield((uint32_t)info->stack + PAGE_SIZE - 4);
    }

    extern "C" void task_yield_done(uint32_t sp) {
        cpu::Status *info;
        Thread *current;
        Thread *next;
        info = cpu::info();

        info->thread->stack_pointer = sp;
        current = info->thread;
        info->thread = NULL;

        // And then use the "normal" memory map
        vm::table_clear();

        next = _next_task(current);

        task_enter(next);
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

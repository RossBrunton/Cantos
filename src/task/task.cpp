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
#include "structures/list.hpp"
#include "main/asm_utils.hpp"

extern "C" {
    #include "task/asm.h"
}

namespace task {
    const uint8_t _INIT_FLAGS = 0x0;

    shared_ptr<Process> kernel_process;

    static uint32_t process_counter;
    static uint32_t task_counter;

    static list<shared_ptr<Process>> processes;

    static mutex::Mutex waiting_mutex;
    static mutex::Mutex _mutex;

    list<shared_ptr<Thread>> waiting_threads;

    list<Utf8> wchans;
    wchan_t no_wchan;

    static void *_memcpy(void *destination, const void *source, size_t num) {
        size_t i;
        for(i = 0; i < num; i ++) {
            ((char *)destination)[i] = ((char *)source)[i];
        }
        return destination;
    }


    void init() {
        kernel_process = make_shared<Process>(0, 0);
        processes.push_back(kernel_process);
        kernel_process->process_id = 0;
        // All other fields 0 by default

        no_wchan = new_wchan(Utf8("."));
    }

    shared_ptr<Process> get_process(uint32_t id) {
        for(const auto &p : processes) {
            if(p->process_id == id) return p;
        }

        return shared_ptr<Process>();
    }


    Process::Process(uint32_t owner, uint32_t group) : process_id(process_counter++) {}

    shared_ptr<Thread> Process::new_thread(addr_logical_t entry_point) {
        shared_ptr<Process> me = get_process(process_id);
        shared_ptr<Thread> t = (threads.emplace_back(make_shared<Thread>(me, entry_point)), threads.back());
        t->wchan = no_wchan;

        waiting_mutex.lock();
        waiting_threads.push_front(t);
        waiting_mutex.unlock();
        return t;
    }

    shared_ptr<Thread> Process::get_thread(uint32_t id) const {
        for(auto t = threads.cbegin(); t != threads.cend(); t ++) {
            if((*t)->thread_id == id) {
                return *t;
            }
        }
        return shared_ptr<Thread>();
    }

    void Process::remove_thread(uint32_t id) {
        for(auto t = threads.begin(); t != threads.end(); t ++) {
            if((*t)->thread_id == id) {
                threads.erase(t);
                return;
            }
        }
    }


    /**
     * @todo Copy all the objects into the new memory map
     * @todo Get the stack object properly
     */
    Thread::Thread(shared_ptr<Process> process, addr_logical_t entry)
        : process(process), thread_id(++process->thread_counter), task_id(++task_counter), in_use(false) {
        bool kernel = process->process_id == 0;
        uint32_t *sp;
        idt_proc_state_t pstate = {0, 0, 0, 0, 0, 0, 0, 0};
        void *stack_installed;

        _mutex.lock();

        // Create the virtual memory map
        vm = make_unique<vm::Map>(process->process_id, task_id, kernel);

        // Create the stack object
        stack = make_shared<object::EmptyObject>(1, page::PAGE_TABLE_RW, 0, 0);
        stack->generate(0, 1);

        vm->add_object(stack, TASK_STACK_TOP - PAGE_SIZE, 0, 1);

        stack_installed = page::kinstall(stack->pages->page, 0);

        // Initial stack format:
        // [pushad values]
        // entry eip
        sp = (uint32_t *)((addr_logical_t)stack_installed + PAGE_SIZE) - 1;
        *sp = (addr_logical_t)task_end;
        sp --;
        *sp = entry;
        sp --;
        *sp = (addr_logical_t)task_asm_entry_point;
        sp --;
        *sp = _INIT_FLAGS;
        sp -= (sizeof(pstate) / 4);
        _memcpy(sp - (sizeof(pstate) / 4), &pstate, sizeof(pstate));

        stack_pointer = TASK_STACK_TOP - sizeof(void *) * 4 - sizeof(pstate);

        page::kuninstall(stack_installed, stack->pages->page);

        _mutex.unlock();
    }


    Thread::~Thread() {
        // Need to do stuff like free the stack or whatever
        printk("Pop! %p\n", this);
        //panic("!");
    }

    void Thread::end() {
        uint32_t eflags = push_cli();
        _mutex.lock();
        ended = true;
        process->remove_thread(thread_id);
        _mutex.unlock();
        pop_flags(eflags);
    }


    extern "C" void __attribute__((noreturn)) task_enter(shared_ptr<Thread> thread) {
        asm volatile ("cli");
        cpu::Status &info = cpu::info();
        uint32_t stack_pointer = thread->stack_pointer;
        uint32_t mem_base = thread->vm->physical_dir->mem_base;
        info.thread = move(thread);

        // And then hop into it
        task_asm_enter(stack_pointer, mem_base);
    }

    void wait(wchan_t wchan) {
        shared_ptr<Thread> t = get_thread();
        t->wchan = wchan;

        task_yield();
    }

    extern "C" void task_yield() {
        uint32_t eflags = push_cli();
        cpu::Status& info = cpu::info();
        bool in_thread = (bool)info.thread;
        uint32_t stack = (uint32_t)info.stack + PAGE_SIZE;
        pop_flags(eflags);

        // Do nothing if we are not in a task
        if(!in_thread) {
            return;
        }

        // Call the exit function to move over the stack, will call task_yield_done
        task_asm_yield(stack);
    }

    extern "C" void __attribute__((noreturn)) task_yield_done(uint32_t sp) {
        shared_ptr<Thread> current;
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
        if(current->wchan == no_wchan) {
            waiting_threads.push_front(current);
        }
        waiting_mutex.unlock();

        schedule();
    }

    void __attribute__((noreturn)) schedule() {
        shared_ptr<Thread> next;

        asm volatile ("cli");
        while(true) {
            waiting_mutex.lock();
            if(!waiting_threads.empty()) {
                next = waiting_threads.back();
                waiting_threads.pop_back();
                waiting_mutex.unlock();
                break;
            }
            waiting_mutex.unlock();
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
            task_enter(move(next));
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

    extern "C" void __attribute__((noreturn)) task_end_done() {
        shared_ptr<Thread> current = cpu::info().thread;
        cpu::info().thread = nullptr;

        current->end();
        current = nullptr;

        schedule();
    }

    extern "C" void  __attribute__((noreturn)) task_end() {
        asm volatile ("cli");

        cpu::Status& info = cpu::info();
        uint32_t stack = (uint32_t)info.stack + PAGE_SIZE;

        // Call the exit function to move over the stack, will call task_yield_done
        task_asm_set_stack(stack, &task_end_done);
        schedule();
    }

    bool in_thread() {
        bool to_ret = false;

        uint32_t eflags = push_cli();
        cpu::Status& info = cpu::info();
        to_ret = (bool)info.thread;
        pop_flags(eflags);

        return to_ret;
    }

    shared_ptr<Thread> get_thread() {
        shared_ptr<Thread> to_ret;

        uint32_t eflags = push_cli();
        cpu::Status& info = cpu::info();
        to_ret = info.thread;
        pop_flags(eflags);

        return to_ret;
    }

    wchan_t new_wchan(Utf8 name) {
        wchans.push_back(move(name));
        return wchans.size() - 1;
    }

    void resume(shared_ptr<Thread> thread) {
        _mutex.lock();
        if(thread->wchan != no_wchan) {
            thread->wchan = no_wchan;
            _mutex.unlock();
            waiting_mutex.lock();
            waiting_threads.push_front(thread);
            waiting_mutex.unlock();
        }else{
            _mutex.unlock();
        }
    }
}

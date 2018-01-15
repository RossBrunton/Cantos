#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include "structures/mutex.hpp"
#include "main/errno.h"
#include "main/printk.hpp"
#include "main/cpu.hpp"
#include "task/task.hpp"
#include "main/asm_utils.hpp"

namespace mutex {
    Mutex::Mutex() {
        this->flag = false;
    }

    int Mutex::lock() {
        uint32_t eflags = push_flags();
        while(true) {
            int result = this->trylock();

            if(result == EOK) {
                return EOK;
            }

            if(eflags & cpu::IF) {
                // Interrupts are enabled, so assume that we can freely halt and get interrupted and stuff
                asm volatile ("cli");
                uint32_t id = cpu::id();
                if(cpu::info_of(id).thread) {
                    asm volatile ("sti");
                    task::task_yield();
                }else{
                    asm volatile ("sti");
                    asm volatile ("hlt");
                }
            }else{
                // Interrupts are disabled, so assume that the user doesn't want any interrupt handlers to run
                // Busy wait, not sure if there is a better way of doing this...
            }
        }
    }

    int Mutex::trylock() {
        volatile bool swap = true;

        asm volatile ("xchg %0, %1" : "+r" (swap), "+m" (flag) : "r" (swap), "m" (flag));

        if(swap) {
            return EBUSY;
        }

        return EOK;
    }

    int Mutex::unlock() {
        flag = false;

        return EOK;
    }
}

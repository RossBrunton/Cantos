#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include "structures/mutex.hpp"
#include "main/errno.h"
#include "main/printk.hpp"
#include "main/cpu.hpp"
#include "task/task.hpp"

namespace mutex {
    Mutex::Mutex() {
        this->flag = false;
    }

    int Mutex::lock() {
        while(true) {
            int result = this->trylock();

            if(result == EOK) {
                return EOK;
            }

            if(task::in_thread()) {
                task::task_yield();
            }else{
                asm volatile ("hlt");
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

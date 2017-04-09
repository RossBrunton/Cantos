#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include "structures/mutex.hpp"
#include "main/errno.h"
#include "main/printk.h"
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
        }
    }

    int Mutex::trylock() {
        volatile bool swap = true;

        asm volatile ("xchg %0, %1" : "+r" (swap), "+m" (this->flag) : "r" (swap), "m" (this->flag));

        if(swap) {
            return EBUSY;
        }

        return EOK;
    }

    int Mutex::unlock() {
        this->flag = false;

        return EOK;
    }
}

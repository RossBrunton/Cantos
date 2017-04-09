#ifndef _H_STRUCTURES_MUTEX_
#define _H_STRUCTURES_MUTEX_


#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "main/errno.h"

namespace mutex {
    class Mutex {
    public:
        Mutex();
        
        int lock();
        int trylock();
        int unlock();
    
    private:
        volatile bool flag;
    };
}

#endif

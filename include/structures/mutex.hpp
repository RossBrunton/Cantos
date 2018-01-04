#ifndef _H_STRUCTURES_MUTEX_
#define _H_STRUCTURES_MUTEX_


#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "main/errno.h"

/** Contains a thread mutex
 */
namespace mutex {
    /** Implementation of a mutex, supporting lock, trylock and unlock.
     *
     * These mutexes must only be used in a thread.
     */
    class Mutex {
    public:
        /** Create a new Mutex */
        Mutex();

        /** Gets a lock on the mutex, or blocks
         *
         * If the mutex is already locked, this blocks until the lock is released.
         *
         * TODO: This is currently implemented as a while(true) loop, it should be changed to thread yielding.
         *
         * @return EOK when we get the lock
         */
        int lock();
        /** Gets a lock on the mutex, or returns EBUSY
         *
         * @return EOK when the mutex is locked or EBUSY if is already locked
         */
        int trylock();
        /** Unlock a previously locked mutex
         *
         * @return EOK
         */
        int unlock();

    private:
        volatile bool flag;
    };
}

#endif

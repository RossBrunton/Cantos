#ifndef _HPP_STRUCT_STATIC_LIST_
#define _HPP_STRUCT_STATIC_LIST_

#include <stddef.h>

#include "main/cpp.hpp"
#include "structures/unique_ptr.hpp"
#include "structures/iterator.hpp"
#include "structures/mutex.hpp"

/** Implements a statically allocated list over the given type that can hold up to C elements.
 *
 * This holds up to the given amount of elements, and is implemented as a cyclic buffer. That means elements can be
 *  pushed or popped from either end freely and quickly.
 *
 * Since the array is statically allocated, no operations will invoke a kmalloc (except if a constructor uses it), at
 *  the cost of space.
 *
 * All operations on this class are thread safe, and some are re-entrant. Should two operations appear to happen at the
 *  same time, it will seem as they all happened at once. Please note that this may involve disabling and re-enabling
 *  interrupts.
 */
template<class T, size_t C> class StaticList {
private:
    T objects[C];
    volatile size_t head = 0;
    volatile size_t tail = 0;
    mutex::Mutex mutex;

public:
    /** Constructs a new, empty static list */
    StaticList() {};

    /** Returns a reference to the frontmost element of the list
     *
     * This function is re-entrant.
     *
     * @return The front element
     */
    T& front() {
        return objects[head];
    }

    /** Returns a reference to the backmost element of the list
     *
     * This function is re-entrant.
     *
     * @return The back element
     */
    T& back() {
        size_t t = tail;
        return objects[t ? t - 1 : C - 1];
    }

    /** Returns true iff the list has no elements
     *
     * This function is re-entrant
     *
     * @return Whether the list is empty
     */
    bool empty() const {
        return head == tail;
    }

    /** Returns the number of items in the list
     *
     * @return How many items are in this list
     */
    size_t size() {
        size_t ret;
        size_t h = head;
        size_t t = tail;

        if(h <= t) {
            ret = t - h;
        }else{
            ret = (C + t) - h;
        }

        return ret;
    }

    /** Deletes all items in the list
     *
     * After this call, the list will be empty.
     */
    void clear() {
        mutex.lock();
        head = tail;
        mutex.unlock();
    }

    /** Push an item to the front of the list
     *
     * The element will be copied.
     *
     * This function is re-entrant.
     *
     * @param value The item to add
     */
    void push_front(const T& value);
    /** Push an item to the front of the list
     *
     * The element will be moved.
     *
     * This function is re-entrant.
     *
     * @param value The item to add
     */
    void push_front(T&& value);
    /** Remove the front item from the list
     *
     * The item will be deleted, and the new front will be the item following it.
     *
     * This function is re-entrant.
     */
    T pop_front();
    /** Construct a new item at the front of the list
     *
     * This function is re-entrant.
     *
     * @param args The arguments for the newly created item's constructor
     */
    template<class... Args> void emplace_front(Args&&... args);

    /** Push an item to the back of the list
     *
     * The element will be copied.
     *
     * This function is re-entrant.
     *
     * @param value The item to add
     */
    void push_back(const T& value);
    /** Push an item to the back of the list
     *
     * The element will be moved.
     *
     * This function is re-entrant.
     *
     * @param value The item to add
     */
    void push_back(T&& value);
    /** Remove the back item from the list
     *
     * This function is re-entrant.
     *
     * The item will be deleted, and the new back will be the item preceding it.
     */
    T pop_back();
    /** Construct a new item at the back of the list
     *
     * @param args The arguments for the newly created item's constructor
     */
    template<class... Args> void emplace_back(Args&&... args);

    /** Return the total number of elements which may be stored in this structure
     *
     * This function is re-entrant.
     */
    size_t capacity() const {
        return C;
    }
};

#include "structures/static_list.tpp"
#endif

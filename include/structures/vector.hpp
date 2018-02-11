#ifndef _HPP_STRUCT_VECTOR_
#define _HPP_STRUCT_VECTOR_

#include <stddef.h>

#include "main/common.hpp"

namespace kmem {
void *kmalloc(size_t size, uint8_t flags);
void kfree(void *ptr);
}

namespace vector_ns {
/** Implements a vector over a specified type, allowing dynamically sized storage
 *
 * This functions similarly to std::vector, except that elements cannot be appended to the front and reserving/shrinking
 *  the buffer on demand is not supported yet (@TODO). In addition, you may only remove elements from the end of the
 *  vector.
 *
 * For those unfamiliar with vectors, they are stored in a continuous memory region, which may grow or shrink as
 *  elements are added or removed. For this reason, several operations may reallocate the buffer, destroying any
 *  existing references.
 *
 * Adding or removing elements is not thread safe.
 *
 * When an object is removed from the vector, their default deconstructor is called, and any references/pointers to them
 *  are invalid. Deleting an element does not invalidate any references/pointers to other elements or the vector itself.
 *
 * @TODO This needs to be thread safe
 */
template<class T> class vector {
private:
    T *buff = nullptr;
    size_t count = 0;
    size_t buff_size = 0;

    void grow_buff(size_t target) {
        T *new_buff = (T *)kmem::kmalloc(sizeof(T) * target, 0);

        for(size_t i = 0; i < count; i ++) {
            new (&(new_buff[i])) T(move(buff[i]));
        }

        kmem::kfree(buff);
        buff = new_buff;
        buff_size = target;
    }

    void grow_buff() {
        grow_buff(buff_size * 2);
    }

public:
    /** Constructs a new, empty vector */
    vector() {
        grow_buff(4);
    };

    /** Constructs a vector of `count` default-constructed elements
     *
     * @param count The number of default constructed elements to create
     */
    vector(size_t count) {
        grow_buff(count);
        for(size_t i = 0; i < count; i ++) {
            new (&(buff[i])) T;
        }
        this->count = count;
    }

    /** Copy constructs a vector, copying all elements from the other buffer into this */
    vector(const vector &other) {
        grow_buff(other.buff_size);

        count = other.count;
        for(size_t i = 0; i < count; i ++) {
            new (&(buff[i])) T(other[i]);
        }
    }

    /** Move constructs another vector, moving the buffer into this */
    vector(vector &&other) : buff(move(other.buff)), count(other.count), buff_size(other.buff_size) {
        other.buff = nullptr;
    }

    /** Copies all elements from the other buffer into this */
    vector &operator=(const vector &other) {
        clear();

        grow_buff(other.buff_size);

        count = other.count;
        for(size_t i = 0; i < count; i ++) {
            new (buff[i]) T(other[i]);
        }

        return *this;
    }

    /** Moves the other buffer into this */
    vector &operator=(vector &&other) {
        clear();

        buff = move(other.buff);
        count = other.count;
        buff_size = other.buff_size;
        other.buff = nullptr;

        return *this;
    }

    ~vector() {
        clear();
        kmem::kfree(buff);
    }

    /** Returns a reference to the frontmost element of the vector
     *
     * @return The front element
     */
    T& front() const {
        return (*this)[0];
    }

    /** Returns a reference to the backmost element of the vector
     *
     * @return The back element
     */
    T& back() const {
        return (*this)[count - 1];
    }

    /** Returns true iff the vector has no elements
     *
     * @return Whether the vector is empty
     */
    bool empty() const {
        return count == 0;
    }

    /** Returns the number of items in the vector
     *
     * @return How many items are in this vector
     */
    size_t size() const {
        return count;
    }

    /** Returns the maximum number of items that can be stored in the vector without it growing
     *
     * This may increase at will
     *
     * @return The current maximum size of the vector
     */
    size_t capacity() const {
        return buff_size;
    }

    /** Deletes all items in the vector
     *
     * After this call, the vector will be empty.
     */
    void clear() {
        if(!buff) return;
        for(T &e : *this) {
            e.~T();
        }
        count = 0;
    }

    /** Accesses the data at the given location
     *
     * @param pos The position to access
     * @return The data at that position
     */
    T &operator[](size_t pos) const {
        if(pos >= count) panic("Out of bounds vector access!");
        if(!buff) panic("Vector is no longer available.");
        return buff[pos];
    }

    /** Accesses the data at the given location
     *
     * @param pos The position to access
     * @return The data at that position
     */
    T &at(size_t pos) const {
        return (*this)[pos];
    }

    /** Returns a pointer to the underlying data
     *
     * @return The underlying data
     */
    T *data() const {
        return buff;
    }

    /** Push an item to the back of the vector
     *
     * The element will be copied, and the underlying buffer may be replaced.
     *
     * @param value The item to add
     */
    void push_back(const T& value) {
        if(count == buff_size) grow_buff();
        new (&((*this)[count ++])) T(value);
    }
    /** Push an item to the back of the vector
     *
     * The element will be moved, and the underlying buffer may be replaced.
     *
     * @param value The item to add
     */
    void push_back(T&& value) {
        if(count == buff_size) grow_buff();
        new (&((*this)[count ++])) T(move(value));
    }
    /** Remove the back item from the vector
     *
     * The item will be deleted, and the new back will be the item preceding it.
     */
    void pop_back() {
        back().~T();
        count --;
    }
    /** Construct a new item at the back of the vector
     *
     * The underlying buffer may be replaced.
     *
     * @param args The arguments for the newly created item's constructor
     */
    template<class... Args> void emplace_back(Args&&... args) {
        if(count == buff_size) grow_buff();
        new (&((*this)[count ++])) T(forward<Args>(args)...);
    }

    /** Returns an iterator to the first element of the vector
     *
     * This iterator will start at the front of the vector, and advance through all the items in order.
     *
     * Modifying an item is allowed, as is appending a new item (whether this item gets iterated through is undefined).
     *  Removing an item other than the current item is also allowed, and it will not be iterated through if it has not
     *  been seen yet. Removing the current item is not allowed.
     *
     * @return An iterater through the vector's contents
     */
    T *begin() const {
        return buff;
    }
    /** Returns a sentinel value that represents the end of an iteration
     *
     * Do not attempt to use this as an iterated value.
     *
     * @return A null iterator
     */
    T *end() const {
        return buff + count;
    }
    /** Returns a constant iterator to the first element of the vector
     *
     * This iterator will start at the front of the vector, and advance through all the items in order.
     *
     * @return An iterater through the vector's contents
     */
    const T *cbegin() const {
        return buff;
    };
    /** Returns a sentinel value that represents the end of a constant iteration
     *
     * Do not attempt to use this as an iterated value.
     *
     * @return A null iterator
     */
    const T *cend() const {
        return buff + count;
    }
};
}
#endif

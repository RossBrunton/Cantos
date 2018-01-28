#ifndef _HPP_STRUCT_UNIQUE_PTR_
#define _HPP_STRUCT_UNIQUE_PTR_

#include <stddef.h>

#include "main/cpp.hpp"
#include "main/panic.hpp"

namespace unique_ptr_ns {
/** A smart pointer that owns and manages an object, deleting it when unique_ptr goes out of scope
 *
 * This is an implementation of unique_ptr from C++11, with the following differences:
 * * Managing an array of objects is not yet supported.
 * * A custom deleter is not yet supported.
 * * Pointer comparsions are not yet supported.
 */
template<class T> class unique_ptr {
private:
    T* ref;

public:
    /** Create a new empty unique_ptr
     */
    unique_ptr() : ref(nullptr) {};
    /** Create a new unique_ptr owning `ref`
     *
     * @param ref The pointer to own
     */
    unique_ptr(T *ref) : ref(ref) {};
    /** Create a new unique_ptr from the given unique_ptr
     *
     * After construction, the other pointer will be empty.
     *
     * @param other The other unique_ptr to steal from.
     */
    unique_ptr(unique_ptr<T>&& other) : ref(other.ref) {
        other.ref = nullptr;
    };

    /** Deletes the associated object, if it owns one
     */
    ~unique_ptr() {
        if(ref) {
            delete ref;
        }
    }

    /** Copy the object (if any) from the other unique_ptr
     *
     * The other unique_ptr will be empty after this call.
     *
     * Any existing object owned by this unique_ptr will be deleted.
     */
    template<class E> unique_ptr& operator=(unique_ptr<E>&& r);
    /** Copy the object (if any) from the other unique_ptr
     *
     * The other unique_ptr will be empty after this call.
     *
     * Any existing object owned by this unique_ptr will be deleted.
     */
    unique_ptr& operator=(unique_ptr&& r);
    /** Clears the object managed by this unique_ptr.
     *
     * It will be deleted.
     */
    unique_ptr& operator=(cpp::nullptr_t r);
    /** Compare two unique_ptrs for equality
     *
     * Two unique_ptrs are equal if they own the same object
     */
    bool operator==(unique_ptr& r) {
        return r.ref == ref;
    }
    /** Compare two unique_ptrs for inequality
     *
     * Two unique_ptrs are not equal if they own different objects
     */
    bool operator!=(unique_ptr& r) {
        return r.ref != ref;
    }

    /** Returns true iff this unique_ptr is not empty
     */
    operator bool() const {
        return ref != nullptr;
    }

    /** Stops managing the managed object, and returns it
     *
     * The unique_ptr will be empty after this call.
     *
     * @return The managed object.
     */
    T *release();
    /** Changes the object managed by this unique_ptr
     *
     * Any existing managed object will be deleted.
     *
     * @param ptr The new object to set
     */
    void reset(T *ptr);
    /** Swaps managed objects with the provided unique_ptr
     *
     * @param other The unique_ptr to swap with.
     */
    void swap(unique_ptr<T>& other);

    /** Returns the managed object */
    T *get() const {
        return ref;
    }

    /** Dereferences the managed object */
    typename cpp::add_lvalue_reference<T>::type operator*() const {
        if(!ref) {
            panic("Tried to dereference a null unique_ptr");
        }
        return *get();
    }

    /** Returns a reference to element idx in an array */
    T& operator[](ptrdiff_t idx) const {
        if(!ref) {
            panic("Tried to dereference a null unique_ptr");
        }
        return *(ref + idx);
    }

    /** Dereferences the managed object */
    T *operator->() const {
        if(!ref) {
            panic("Tried to dereference a null unique_ptr");
        }
        return get();
    }
};

/** Creates a unique_ptr managing a newly created and allocated T
 *
 * The constructor for T will be called as appropriate depending on `args`.
 *
 * @param args The arguments to pass through to T's constructor.
 * @return A unique_ptr to the newly allocated object.
 */
template<class T, class... Args> unique_ptr<T> make_unique(Args&&... args);

#include "structures/unique_ptr.tpp"
}
#endif

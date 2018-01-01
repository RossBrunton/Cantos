#ifndef _HPP_STRUCT_SHARED_PTR_
#define _HPP_STRUCT_SHARED_PTR_

#include <stddef.h>

#include "main/cpp.hpp"
#include "main/panic.hpp"

/** A smart pointer where multiple own and manage a single object, deleting it when all shared_ptrs goes out of scope
 *
 * Multiple shared_ptrs can own the same object, and the object is deleted only when there are no more shared_ptrs
 *  owning it (e.g., when the last one is deleted or falls out of scope).
 *
 * This is an implementation of shared_ptr from C++11, with the following differences:
 * * A custom deleter is not yet supported.
 * * Pointer comparsions are not yet supported.
 */
template<class T> class shared_ptr {
private:
    struct Data {
        uint32_t uses = 1;
    };

    T *ref;
    Data *data;

    void decrement_usage(); // Also deletes if appropriate

public:
    /** Create a new empty shared_ptr
     */
    shared_ptr() : ref(nullptr), data(nullptr) {};
    /** Create a new shared_ptr owning `ref`
     *
     * @param ref The pointer to own
     */
    shared_ptr(T *ref);
    /** Create a new shared_ptr managing the shared resource from the given shared_ptr
     *
     * @param other The other shared_ptr to share from
     */
    shared_ptr(shared_ptr<T>& other);
    /** Create a new shared_ptr from the given shared_ptr
     *
     * After construction, the other pointer will be empty.
     *
     * @param other The other shared_ptr to steal from
     */
    shared_ptr(shared_ptr<T>&& other);

    /** Deletes the associated object, if no other shared_ptrs own it
     */
    ~shared_ptr();

    /** Get the object (if any) from the other shared_ptr, and share it with this shared_ptr
     *
     * Any existing object owned by this shared_ptr will be deleted if this is the last shared_ptr owning it.
     */
    shared_ptr& operator=(shared_ptr& r);
    /** Copy the object (if any) from the other shared_ptr
     *
     * The other shared_ptr will be empty after this call.
     *
     * Any existing object owned by this shared_ptr will be deleted if this is the last shared_ptr owning it.
     */
    shared_ptr& operator=(shared_ptr&& r);
    /** Clears the object managed by this shared_ptr.
     *
     * It will be deleted if no other shared_ptrs own it.
     */
    shared_ptr& operator=(nullptr_t r);

    /** Returns true iff this shared_ptr is not empty
     */
    operator bool() const {
        return ref != nullptr;
    }

    /** Changes the object managed by this shared_ptr
     *
     * Any existing managed object will be deleted if this is the last shared_ptr owning it.
     *
     * @param ptr The new object to set
     */
    void reset(T *ptr);
    /** Swaps managed objects with the provided shared_ptr
     *
     * @param other The shared_ptr to swap with.
     */
    void swap(shared_ptr<T>& other);
    /** Returns the number of shared_ptrs sharing the resource */
    uint32_t use_count() {
        return data->uses;
    }

    /** Returns the managed object */
    T *get() const {
        return ref;
    }

    /** Dereferences the managed object */
    typename add_lvalue_reference<T>::type operator*() const {
        if(!ref) {
            panic("Tried to dereference a null shared_ptr");
        }
        return *get();
    }

    /** Returns a reference to element idx in an array */
    T& operator[](ptrdiff_t idx) const {
        if(!ref) {
            panic("Tried to dereference a null shared_ptr");
        }
        return *(ref + idx);
    }

    /** Dereferences the managed object */
    T *operator->() const {
        if(!ref) {
            panic("Tried to dereference a null shared_ptr");
        }
        return get();
    }
};

/** Creates a shared_ptr managing a newly created and allocated T
 *
 * The constructor for T will be called as appropriate depending on `args`.
 *
 * @param args The arguments to pass through to T's constructor.
 * @return A shared_ptr to the newly allocated object.
 */
template<class T, class... Args> shared_ptr<T> make_shared(Args&&... args);

#include "structures/shared_ptr.tpp"
#endif

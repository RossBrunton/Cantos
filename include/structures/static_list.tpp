#include "main/cpp.hpp"
#include "test/test.hpp"
#include "main/panic.hpp"
#include "main/asm_utils.hpp"

template<class T, size_t C> void StaticList<T, C>::push_front(const T& value) {
    uint32_t eflags = push_cli();
    mutex.lock();
    head = head ? head - 1 : C - 1;
    objects[head] = value;
    mutex.unlock();
    pop_flags(eflags);
}

template<class T, size_t C> void StaticList<T, C>::push_front(T&& value) {
    uint32_t eflags = push_cli();
    mutex.lock();
    head = head ? head - 1 : C - 1;
    objects[head] = value;
    mutex.unlock();
    pop_flags(eflags);
}

template<class T, size_t C> T StaticList<T, C>::pop_front() {
    uint32_t eflags = push_cli();
    mutex.lock();
    if(head == tail) {
        panic("Tried to pop_front an empty list");
    }

    T old_v;
    old_v = objects[head];
    head = (head + 1) % C;
    mutex.unlock();
    pop_flags(eflags);

    return old_v;
}

template<class T, size_t C> template<class... Args> void StaticList<T, C>::emplace_front(Args&&... args) {
    uint32_t eflags = push_cli();
    mutex.lock();
    head = head ? head - 1 : C - 1;
    objects[head] = T(forward<Args>(args)...);
    mutex.unlock();
    pop_flags(eflags);
}


template<class T, size_t C> void StaticList<T, C>::push_back(const T& value) {
    uint32_t eflags = push_cli();
    mutex.lock();
    objects[tail] = value;
    tail = (tail + 1) % C;
    mutex.unlock();
    pop_flags(eflags);
}

template<class T, size_t C> void StaticList<T, C>::push_back(T&& value) {
    uint32_t eflags = push_cli();
    mutex.lock();
    objects[tail] = value;
    tail = (tail + 1) % C;
    mutex.unlock();
    pop_flags(eflags);
}

template<class T, size_t C> T StaticList<T, C>::pop_back() {
    uint32_t eflags = push_cli();
    mutex.lock();
    if(head == tail) {
        panic("Tried to pop_back an empty list");
    }

    tail = tail ? tail - 1 : C - 1;
    T old_v = objects[tail];
    mutex.unlock();
    pop_flags(eflags);

    return old_v;
}

template<class T, size_t C> template<class... Args> void StaticList<T, C>::emplace_back(Args&&... args) {
    uint32_t eflags = push_cli();
    mutex.lock();
    objects[tail] = T(forward<Args>(args)...);
    tail = (tail + 1) % C;
    mutex.unlock();
    pop_flags(eflags);
}

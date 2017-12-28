#include "main/cpp.hpp"
#include "test/test.hpp"
#include "main/panic.hpp"

template<class T> list<T>::list(size_t count) {
    while(count--) {
        emplace_front();
    }
}

template<class T> void list<T>::push_front(const T& value) {
    unique_ptr<Entry> append = make_unique<Entry>(value);

    if(head) {
        append->next = move(head);
        head = move(append);
    }else{
        head = move(append);
        tail = head.get();
    }
    count ++;
}

template<class T> void list<T>::push_front(T&& value) {
    unique_ptr<Entry> append = make_unique<Entry>(move(value));

    if(head) {
        append->next = move(head);
        head = move(append);
    }else{
        head = move(append);
        tail = head.get();
    }
    count ++;
}

template<class T> void list<T>::pop_front() {
    if(!head) {
        panic("Tried to pop_front an empty list");
    }

    unique_ptr<Entry> old_head = move(head);
    head = move(old_head->next);

    if(!head) {
        tail = nullptr;
    }

    count --;
}

template<class T> template<class... Args> void list<T>::emplace_front(Args&&... args) {
    unique_ptr<Entry> append = make_unique<Entry>(T(forward<Args>(args)...));

    if(head) {
        append->next = move(head);
        head = move(append);
    }else{
        head = move(append);
        tail = head.get();
    }
    count ++;
}


template<class T> void list<T>::push_back(const T& value) {
    unique_ptr<Entry> append = make_unique<Entry>(value);

    if(head) {
        tail->next = move(append);
        tail = tail->next.get();
    }else{
        head = move(append);
        tail = head.get();
    }
    count ++;
}

template<class T> void list<T>::push_back(T&& value) {
    unique_ptr<Entry> append = make_unique<Entry>(move(value));

    if(head) {
        tail->next = move(append);
        tail = tail->next.get();
    }else{
        head = move(append);
        tail = head.get();
    }
    count ++;
}

template<class T> void list<T>::pop_back() {
    if(!head) {
        panic("Tried to pop_back an empty list");
    }

    if(head.get() == tail) {
        head = nullptr;
        tail = nullptr;
        return;
    }

    Entry *e;
    for(e = head.get(); e->next.get() != tail; e = e->next.get()) {}

    tail = e;
    e->next = nullptr;

    count --;
}

template<class T> template<class... Args> void list<T>::emplace_back(Args&&... args) {
    unique_ptr<Entry> append = make_unique<Entry>(T(forward<Args>(args)...));

    if(head) {
        tail->next = move(append);
        tail = tail->next.get();
    }else{
        head = move(append);
        tail = head.get();
    }

    count ++;
}

template<class T> void list<T>::clear() {
    head = nullptr;
    tail = nullptr;
    count = 0;
}

template<class T> typename list<T>::Iterator list<T>::begin() {
    return Iterator(head.get());
}

template<class T> typename list<T>::Iterator list<T>::end() {
    return Iterator(nullptr);
}

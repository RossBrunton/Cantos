#include "main/cpp.hpp"

template<class T> shared_ptr<T>::shared_ptr(T *ref) : ref(ref) {
    if(ref) {
        data = new Data();
    }
}

template<class T> shared_ptr<T>::shared_ptr(const shared_ptr<T> &other) : ref(other.ref), data(other.data) {
    if(ref) {
        data->uses ++;
    }
}

template<class T> shared_ptr<T>::shared_ptr(shared_ptr<T> &&other) : ref(other.ref), data(other.data) {
    other.ref = nullptr;
    other.data = nullptr;
}

template<class T> shared_ptr<T>::~shared_ptr() {
    decrement_usage();
}

template<class T> void shared_ptr<T>::decrement_usage() {
    if(ref) {
        data->uses --;
        if(!data->uses) {
            delete ref;
            delete data;
        }
    }
}

template<class T> shared_ptr<T>& shared_ptr<T>::operator=(shared_ptr<T>& r) {
    if(r.ref) {
        r.data->uses ++;
    }
    decrement_usage();

    ref = r.ref;
    data = r.data;

    return *this;
}

template<class T> shared_ptr<T>& shared_ptr<T>::operator=(shared_ptr<T>&& r) {
    decrement_usage();
    ref = r.ref;
    r.ref = nullptr;
    data = r.data;
    r.data = nullptr;

    return *this;
}

template<class T> shared_ptr<T>& shared_ptr<T>::operator=(nullptr_t r) {
    decrement_usage();
    ref = nullptr;
    data = nullptr;

    return *this;
}

template<class T> void shared_ptr<T>::reset(T *ptr) {
    decrement_usage();

    ref = ptr;
    data = new Data();
}

template<class T> void shared_ptr<T>::swap(shared_ptr<T>& other) {
    T *otherRef = other.ref;
    other.ref = ref;
    ref = otherRef;

    Data *otherData = other.data;
    other.data = data;
    data = otherData;
}


template<class T, class... Args> shared_ptr<T> make_shared(Args&&... args) {
    return shared_ptr<T>(new T(forward<Args>(args)...));
}

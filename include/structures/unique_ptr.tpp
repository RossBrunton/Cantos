#include "main/cpp.hpp"

template<class T> template<class E> unique_ptr<T>& unique_ptr<T>::operator=(unique_ptr<E>&& r) {
    T *oldRef = ref;

    ref = r.release();

    if(oldRef) {
        delete oldRef;
    }

    return *this;
}

template<class T> unique_ptr<T>& unique_ptr<T>::operator=(unique_ptr<T>&& r) {
    T *oldRef = ref;

    ref = r.release();

    if(oldRef) {
        delete oldRef;
    }

    return *this;
}

template<class T> unique_ptr<T>& unique_ptr<T>::operator=(nullptr_t r) {
    if(ref) {
        delete ref;
    }
    ref = nullptr;

    return *this;
}

template<class T> T *unique_ptr<T>::release() {
    T *old = ref;
    ref = nullptr;
    return old;
}

template<class T> void unique_ptr<T>::reset(T *ptr) {
    if(ref) {
        delete ref;
    }

    ref = ptr;
}

template<class T> void unique_ptr<T>::swap(unique_ptr<T>& other) {
    T *otherRef = other.ref;
    other.ref = ref;
    ref = otherRef;
}


template<class T, class... Args> unique_ptr<T> make_unique(Args&&... args) {
    return unique_ptr<T>(new T(forward<Args>(args)...));
}

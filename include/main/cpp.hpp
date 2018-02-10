#ifndef _HPP_CPP_
#define _HPP_CPP_

#include <stddef.h>

// Destructors (do nothing)
namespace cpp {
extern "C" int __cxa_atexit(void (*destructor) (void *), void *arg, void *dso);
extern "C" void __cxa_finalize(void *f);

typedef decltype(nullptr) nullptr_t;

template <class T, T v>
struct integral_constant {
    static constexpr T value = v;
    typedef T value_type;
    typedef integral_constant<T,v> type;
    constexpr operator T() {return v;}
};
typedef integral_constant<bool, false> false_type;
typedef integral_constant<bool, true> true_type;

template<class T> struct add_lvalue_reference {typedef T& type;};
template<class T> struct add_lvalue_reference<T&&> {typedef T& type;};

template<class T> struct is_lvalue_reference : false_type {};
template<class T> struct is_lvalue_reference<T&> : true_type {};

template<class T> struct remove_reference {typedef T type;};
template<class T> struct remove_reference<T&> {typedef T type;};
template<class T> struct remove_reference<T&&> {typedef T type;};

template<class T> typename remove_reference<T>::type&& move(T&& t) {
    return static_cast<typename remove_reference<T>::type&&>(t);
}

template<typename T> T&& forward(typename remove_reference<T>::type& x) {
    return static_cast<T&&>(x);
}
}

void *operator new(size_t size, void *pos);
void *operator new[](size_t size, void *pos);
#endif

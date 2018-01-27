#ifndef _HPP_STRUCT_FAILABLE_
#define _HPP_STRUCT_FAILABLE_

#include <stddef.h>

#include "main/common.hpp"
#include "main/errno.h"

namespace failable {
template<class T> class Failable {
public:
    error_t err;
    T val;

    Failable(error_t err) : err(err), val(T()) {}
    Failable(error_t err, T val) : err(err), val(val) {}
    Failable(Failable &other) : err(other.err), val(other.val) {}
    Failable(Failable &&other) : err(other.err), val(move(other.val)) {}

    explicit operator bool() const {
        return err == EOK;
    }
};
}
#endif

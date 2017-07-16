#ifndef _HPP_STRUCTURES_UTF8_
#define _HPP_STRUCTURES_UTF8_

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

namespace utf8 {
    class Utf8 {
    public:
        Utf8(const char *string);
        Utf8(const char *string, bool autofree);
        ~Utf8();
        const char *to_string() const;

        uint32_t bytes() const; // Does not include the terminating null
        uint32_t chars() const;

        bool operator==(const Utf8 &other) const;
        Utf8 operator+(const Utf8 &other);

    private:
        const char *string;
        bool autofree = false;
    };
}

using utf8::Utf8;

#endif

#ifndef _HPP_STRUCTURES_UTF8_
#define _HPP_STRUCTURES_UTF8_

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

namespace utf8 {
    class Utf8 {
    public:
        Utf8(const char *string, bool autofree=false);
        Utf8(const Utf8 &copy);
        ~Utf8();
        const char *to_string() const;

        uint32_t bytes() const; // Does not include the terminating null
        uint32_t chars() const;
        Utf8 substr(size_t pos, size_t len) const;

        int compare(Utf8 str) const;
        int compare(size_t pos, size_t len, Utf8 str) const;
        int compare(size_t pos, size_t len, Utf8 str, size_t subpos, size_t sublen) const;

        size_t find(Utf8 str, size_t pos = 0) const;
        size_t find(char c, size_t pos = 0) const;

        bool operator==(const Utf8 &other) const;
        Utf8 operator+(const Utf8 &other);
        char operator[] (int x) const;

        static const size_t npos = -1;

    private:
        const char *string;
        bool autofree = false;
        size_t byte_count = 0;
    };
}

using utf8::Utf8;

#endif

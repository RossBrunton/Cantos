#include <stdint.h>
#include <stdbool.h>

#include "mem/kmem.hpp"
#include "structures/utf8.hpp"
#include "structures/shared_ptr.hpp"
#include "test/test.hpp"

namespace utf8 {
    static const char *_empty_string = "";

    inline size_t _strlen(const char *str) {
        if(!str) {
            panic("Null in _strlen");
        }
        size_t p = 0;
        while(p < UINT32_MAX) {
            if(str[p] == '\0') {
                return p;
            }
            p ++;
        }
        return UINT32_MAX - 1;
    }

    Utf8::Utf8(const char *string) : string(string), static_alloc(true), byte_count(_strlen(string)) {}

    Utf8 Utf8::own(const char *buffer, uint32_t size) {
        Utf8 base;

        if(size == npos) {
            // Calculate bytes
            size = _strlen(buffer);
        }

        base.byte_count = size;
        base.string_ptr = shared_ptr<const char>(buffer);
        base.string = buffer;

        return base;
    }

    Utf8::Utf8(Utf8 &copy) : string_ptr(copy.string_ptr), string(copy.string), static_alloc(copy.static_alloc),
        byte_count(copy.byte_count) {}
    Utf8::Utf8(Utf8 &&move) : string_ptr(::move(move.string_ptr)), string(move.string), static_alloc(move.static_alloc),
        byte_count(move.byte_count) {
        move.string = _empty_string;
        move.static_alloc = true;
        move.byte_count = 0;
    }

    Utf8::Utf8() : string_ptr(shared_ptr<const char>(nullptr)), string(_empty_string), static_alloc(true), byte_count(0) {}

    const char *Utf8::to_string() const {
        return string;
    }

    uint32_t Utf8::bytes() const {
        return byte_count;
    }

    Utf8 Utf8::substr(size_t pos, size_t len) const {
        size_t c;

        if(len > bytes() - pos) {
            len = bytes() - pos;
        }

        char *newbuff = (char *)kmem::kmalloc(len + 1, 0);

        for(c = 0; c < len; c ++) {
            newbuff[c] = string[pos + c];
        }

        newbuff[len] = '\0';
        return Utf8::own(newbuff, len);
    }

    uint32_t Utf8::chars() const {
        uint32_t p = 0;
        uint32_t c = 0;
        while(p < UINT32_MAX) {
            if(string[p] == '\0') {
                return c;
            }
            if(!(string[p] & 0x80)) {
                c ++;
            }
            if(string[p] & 0xc0) {
                c ++;
            }
            p ++;
        }
        return c;
    }

    int Utf8::compare(size_t pos, size_t len, const Utf8& str, size_t subpos, size_t sublen) const {
        size_t count = len > sublen ? sublen : len;

        // Check if we are just pointers to the same string
        if(string == str.string) {
            return 0;
        }

        for(size_t i = 0; i < count; i ++) {
            if(string[pos + i] != str[subpos + i]) {
                return string[pos + i] - str[subpos + i];
            }
        }

        if(len > sublen) {
            return 1;
        }else if(sublen > len) {
            return -1;
        }else{
            return 0;
        }
    }

    int Utf8::compare(size_t pos, size_t len, const Utf8& str) const {
        return compare(pos, len, str, 0, str.bytes());
    }

    int Utf8::compare(const Utf8& str) const {
        return compare(0, bytes(), str, 0, str.bytes());
    }

    size_t Utf8::find(const Utf8 &str, size_t pos) const {
        for(; pos <= bytes() - str.bytes(); pos ++) {
            if(!compare(pos, str.bytes(), str)) {
                return pos;
            }
        }

        return Utf8::npos;
    }

    size_t Utf8::find(char c, size_t pos) const {
        for(; pos <= bytes(); pos ++) {
            if(string[pos] == c) {
                return pos;
            }
        }

        return Utf8::npos;
    }

    bool Utf8::operator==(const Utf8 &other) const {
        // Check if we are just pointers to the same string
        if(string == other.string) {
            return true;
        }

        // And whether lengths are correct
        if(bytes() != other.bytes()) {
            return false;
        }

        uint32_t p = 0;
        while(p < UINT32_MAX) {
            if(string[p] != other.string[p]) {
                return false;
            }
            if(string[p] == '\0') {
                return true;
            }
            p ++;
        }
        return true;
    }

    bool Utf8::operator!=(const Utf8 &other) const {
        return !(*this == other);
    }

    bool Utf8::operator==(const char *other) const {
        uint32_t p = 0;
        while(p < UINT32_MAX) {
            if(string[p] != other[p]) {
                return false;
            }
            if(string[p] == '\0') {
                return true;
            }
            p ++;
        }
        return true;
    }

    bool Utf8::operator!=(const char *other) const {
        return !(*this == other);
    }

    Utf8& Utf8::operator=(Utf8 &other) {
        string = other.string;
        string_ptr = other.string_ptr;
        static_alloc = other.static_alloc;
        byte_count = other.byte_count;

        return *this;
    }

    Utf8& Utf8::operator=(Utf8 &&other) {
        string = other.string;
        string_ptr = move(other.string_ptr);
        static_alloc = other.static_alloc;
        byte_count = other.byte_count;

        other.string = _empty_string;
        other.static_alloc = true;
        other.byte_count = 0;

        return *this;
    }

    Utf8 Utf8::operator+(const Utf8 &other) {
        uint32_t sa = bytes();
        uint32_t sb = other.bytes();
        uint32_t st = sa + sb;
        if(UINT32_MAX - 1 - sb < sa || UINT32_MAX - 1 - sa < sb) {
            st = UINT32_MAX - 1;
        }

        char *newbuff = (char *)kmem::kmalloc(sa+sb+1, 0);
        for(uint32_t p = 0; p < sa && (p < UINT32_MAX - 1); p ++) {
            newbuff[p] = string[p];
        }
        for(uint32_t p = 0; p < sb && ((p + sa) < UINT32_MAX - 1); p ++) {
            newbuff[sa+p] = other.string[p];
        }
        newbuff[st] = '\0';

        return Utf8::own(newbuff, st);
    }

    Utf8 Utf8::operator+(const char *other) {
        uint32_t sa = bytes();
        uint32_t sb = _strlen(other);
        uint32_t st = sa + sb;
        if(UINT32_MAX - 1 - sb < sa || UINT32_MAX - 1 - sa < sb) {
            st = UINT32_MAX - 1;
        }

        char *newbuff = (char *)kmem::kmalloc(sa+sb+1, 0);
        for(uint32_t p = 0; p < sa && (p < UINT32_MAX - 1); p ++) {
            newbuff[p] = string[p];
        }
        for(uint32_t p = 0; p < sb && ((p + sa) < UINT32_MAX - 1); p ++) {
            newbuff[sa+p] = other[p];
        }
        newbuff[st] = '\0';

        return Utf8::own(newbuff, st);
    }

    char Utf8::operator[] (int x) const {
        return string[x];
    }
}


namespace _tests {
class Utf8Test : public test::TestCase {
public:
    Utf8Test() : test::TestCase("UTF-8 Test") {};

    void run_test() override {
        test("Constructing - Empty");
        Utf8 a;
        assert(!a.bytes());
        assert(a == Utf8());

        test("Constructing - Char *");
        Utf8 b("B");
        assert(b.bytes() == 1);
        assert(b.to_string()[0] == 'B');
        assert(b != Utf8());

        test("Equality");
        Utf8 b2("B2");
        Utf8 b3("B");
        Utf8 a2("A");
        assert(a != b);
        assert(b != a);
        assert(b != b2);
        assert(b2 != b);
        assert(b == b3);
        assert(b3 == b);
        assert(b != a2);
        assert(a2 != b);

        assert(b == "B");
        assert(b2 == "B2");
        assert(b != "B2");
        assert(a == "");
        assert(b != "");

        test("Concatenation");
        assert(b + "2" == "B2");
        assert(b + "3" != "B2");
        assert(b + b == "BB");

        assert(b + "2" == b2);
    }
};

test::AddTestCase<Utf8Test> utf8Test;
}

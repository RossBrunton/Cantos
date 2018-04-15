#include <stdint.h>
#include <stdbool.h>

#include "main/utils.hpp"
#include "mem/kmem.hpp"
#include "structures/utf8.hpp"
#include "structures/shared_ptr.hpp"
#include "test/test.hpp"

namespace utf8 {
    static const char *_empty_string = "";

    Utf8::Utf8(const char *string) : string(string), byte_count(strlen(string)) {}

    Utf8 Utf8::own(const char *buffer, uint32_t size) {
        Utf8 base;

        if(size == npos) {
            // Calculate bytes
            size = strlen(buffer);
        }

        base.byte_count = size;
        base.string_ptr = shared_ptr<const char>(buffer);
        base.string = buffer;

        return base;
    }

    Utf8::Utf8(const Utf8 &copy) : string_ptr(copy.string_ptr), string(copy.string),
        byte_count(copy.byte_count) {}
    Utf8::Utf8(Utf8 &&move) : string_ptr(::move(move.string_ptr)), string(move.string),
        byte_count(move.byte_count) {
        move.string = _empty_string;
        move.byte_count = 0;
    }

    Utf8::Utf8() : string_ptr(shared_ptr<const char>(nullptr)), string(_empty_string), byte_count(0) {}

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
        byte_count = other.byte_count;

        return *this;
    }

    Utf8& Utf8::operator=(Utf8 &&other) {
        string = other.string;
        string_ptr = move(other.string_ptr);
        byte_count = other.byte_count;

        other.string = _empty_string;
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
        uint32_t sb = strlen(other);
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

    namespace {
        uint8_t num_to_str(uint8_t value, bool uppercase) {
            if (value < 10)
                return value + 0x30;
            if (uppercase)
                return value + 0x41 - 0xa;
            return value + 0x61 - 0xa;
        }
    }

    Utf8 Utf8::format(...) const {
        va_list va;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvarargs"
        // C++ inserts this as the first argument, so we can just use that. This may be a bad idea
        va_start(va, this);
        Utf8 ret = format(va);
        va_end(va);
#pragma clang diagnostic pop
        return ret;
    }

    Utf8 Utf8::format(va_list ap) const {
        vector<uint8_t> out;
        const Utf8& fmt = *this;
        static const int NUM_BUFFER_SIZE = 20;

        for (size_t p = 0; fmt[p]; p++) {
            char cur = fmt[p];

            if (cur == '%') {
                int length = 0;
            start : {
                cur = fmt[++p];
                switch (cur) {
                case '%':
                    out.push_back('%');
                    break;
                case 'n': {
                    (void)0;
                    int* slot = va_arg(ap, int*);
                    *slot = out.size();
                    break;
                }
                case 'l':
                    length++;
                    if (length > 2)
                        length = 2;
                    goto start;
                case 'p':
                    out.push_back('0');
                    out.push_back('x');
                case 'x':
                case 'X':
                case 'o':
                case 'u':
                case 'd':
                case 'i': {
                    int val = 0;
                    if (!length)
                        val = va_arg(ap, unsigned int);
                    if (length == 1)
                        val = va_arg(ap, unsigned long);
                    if (length == 2)
                        val = va_arg(ap, unsigned long long);
                    int base = 10;

                    if (cur == 'x' || cur == 'X' || cur == 'p')
                        base = 16;
                    if (cur == 'o')
                        base = 8;

                    if (cur == 'd' || cur == 'i') {
                        signed int vals = val;
                        if (vals < 0) {
                            val = -vals;
                            out.push_back('-');
                        }
                    }

                    uint8_t output[NUM_BUFFER_SIZE];
                    for (int i = NUM_BUFFER_SIZE - 1; i >= 0; i--) {
                        output[i] = num_to_str(val % base, cur == 'X');
                        val = val / base;
                    }

                    size_t start = 0;
                    while (output[start] == '0' && start < NUM_BUFFER_SIZE - 1) {
                        start++;
                    }

                    for (size_t i = start; i < NUM_BUFFER_SIZE; i++) {
                        out.push_back(output[i]);
                    }

                    break;
                }
                case 's': {
                    (void)0;
                    const char* str = va_arg(ap, const char*);
                    size_t len = strlen(str);
                    for (size_t i = 0; i < len; i++) {
                        out.push_back(str[i]);
                    }
                    break;
                }
                case 'S': {
                    (void)0;
                    Utf8* str = va_arg(ap, Utf8*);
                    size_t len = str->bytes();
                    for (size_t i = 0; i < len; i++) {
                        out.push_back((*str)[i]);
                    }
                    break;
                }
                }
            }
            } else {
                out.push_back(cur);
            }
        }

        va_end(ap);

        char* out_buf = new char[out.size() + 1];
        memcpy(out_buf, out.data(), out.size());
        out_buf[out.size()] = '\0';
        return Utf8::own(out_buf, out.size());
    }
}


namespace _tests {
class Utf8Test : public test::TestCase {
public:
    Utf8Test() : test::TestCase("UTF-8 Test"){};

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

        test("Formatting");
        Utf8 f("%s %s %d %x");
        assert(f.format("Hello", "World", 69, 0xff) == "Hello World 69 ff");
        Utf8 f2("%S %S");
        Utf8 h("Hello");
        Utf8 w("World");
        assert(f2.format(&h, &w) == "Hello World");
    }
};

test::AddTestCase<Utf8Test> utf8Test;
}

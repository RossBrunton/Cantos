#include <stdint.h>
#include <stdbool.h>

#include "mem/kmem.hpp"
#include "structures/utf8.hpp"

namespace utf8 {
    Utf8::Utf8(const char *string) {
        this->string = string;
    }

    Utf8::Utf8(const char *string, bool autofree) {
        this->string = string;
        this->autofree = autofree;
    }

    Utf8::~Utf8() {
        if(this->autofree) {
            kfree((void *)this->string);
        }
    }

    const char *Utf8::to_string() const {
        return this->string;
    }

    uint32_t Utf8::bytes() const {
        uint32_t p = 0;
        while(p < UINT32_MAX) {
            if(this->string[p] == '\0') {
                return p;
            }
            p ++;
        }
        return UINT32_MAX;
    }

    uint32_t Utf8::chars() const {
        uint32_t p = 0;
        uint32_t c = 0;
        while(p < UINT32_MAX) {
            if(this->string[p] == '\0') {
                return c;
            }
            if(!(this->string[p] & 0x80)) {
                c ++;
            }
            p ++;
        }
        return c;
    }

    bool Utf8::operator==(const Utf8 &other) const {
        uint32_t p = 0;
        while(p < UINT32_MAX) {
            if(this->string[p] != other.string[p]) {
                return false;
            }
            if(this->string[p] == '\0') {
                return true;
            }
            p ++;
        }
        return true;
    }

    Utf8 Utf8::operator+(const Utf8 &other) {
        uint32_t sa = this->bytes();
        uint32_t sb = other.bytes();
        uint32_t st = sa + sb;
        if(UINT32_MAX - sb < sa || UINT32_MAX - sa < sb) {
            st = UINT32_MAX;
        }

        char *newbuff = (char *)kmalloc(sa+sb+1, 0);
        for(uint32_t p = 0; p < sa && (p < UINT32_MAX - 1); p ++) {
            newbuff[p] = this->string[p];
        }
        for(uint32_t p = 0; p < sb && ((p + sa) < UINT32_MAX - 1); p ++) {
            newbuff[sa+p] = other.string[p];
        }
        newbuff[st] = '\0';

        return Utf8(newbuff, true);
    }
}

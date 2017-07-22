#include <stdint.h>
#include <stdbool.h>

#include "mem/kmem.hpp"
#include "structures/utf8.hpp"

namespace utf8 {
    Utf8::Utf8(const char *string, bool autofree) {
        this->string = string;
        this->autofree = autofree;

        // Calculate bytes
        uint32_t p = 0;
        this->byte_count = UINT32_MAX;
        while(p < UINT32_MAX) {
            if(this->string[p] == '\0') {
                this->byte_count = p;
                break;
            }
            p ++;
        }
    }

    Utf8::Utf8(const Utf8 &copy) {
        uint8_t c;
        char *buff = (char *)kmalloc(copy.bytes(), 0);

        for(c = 0; copy.string[c] != '\0'; c ++) {
            buff[c] = copy.string[c];
        }

        buff[c] = '\0';
        this->string = buff;
        this->byte_count = copy.byte_count;
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
        return this->byte_count;
    }

    Utf8 Utf8::substr(size_t pos, size_t len) const {
        size_t c;

        if(len > this->bytes() - pos) {
            len = this->bytes() - pos;
        }

        char *newbuff = (char *)kmalloc(len + 1, 0);

        for(c = 0; c < len; c ++) {
            newbuff[c] = this->string[pos + c];
        }

        newbuff[len] = '\0';
        return Utf8(newbuff, true);
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

    int Utf8::compare(size_t pos, size_t len, Utf8 str, size_t subpos, size_t sublen) const {
        size_t count = len > sublen ? sublen : len;

        for(size_t i = 0; i < count; i ++) {
            if(this->string[pos + i] != str[subpos + i]) {
                return this->string[pos + i] - str[subpos + i];
            }
        }

        if(len > sublen) {
            return 1;
        }else if(sublen < len) {
            return -1;
        }else{
            return 0;
        }
    }

    int Utf8::compare(size_t pos, size_t len, Utf8 str) const {
        return this->compare(pos, len, str, 0, str.bytes());
    }

    int Utf8::compare(Utf8 str) const {
        return this->compare(0, this->bytes(), str, 0, str.bytes());
    }

    size_t Utf8::find(Utf8 str, size_t pos) const {
        for(; pos <= this->bytes() - str.bytes(); pos ++) {
            if(!this->compare(pos, str.bytes(), str)) {
                return pos;
            }
        }

        return Utf8::npos;
    }

    size_t Utf8::find(char c, size_t pos) const {
        for(; pos <= this->bytes(); pos ++) {
            if(this->string[pos] == c) {
                return pos;
            }
        }

        return Utf8::npos;
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

    char Utf8::operator[] (int x) const {
        return this->string[x];
    }
}

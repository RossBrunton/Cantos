#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include "main/common.hpp"

#include "structures/stream.hpp"
#include "main/errno.h"

namespace stream {
    /** @todo stream_writef Doesn't handle errors in the stream, or writing less than it expected to.
     */

    error_t Stream::write(const void *buff, size_t len, uint32_t flags, void *data, uint32_t *written) {
        return EPERM;
    }

    error_t Stream::read(void *buff, size_t len, uint32_t flags, void *data, uint32_t *read) {
        return EPERM;
    }

    error_t Stream::peek(void *buff, size_t len, uint32_t flags, void *data, uint32_t *read) {
        return EPERM;
    }

    error_t Stream::skip(size_t len, uint32_t flags, void *data, uint32_t *skipped) {
        return EPERM;
    }

    static uint8_t _num_to_str(uint8_t value, bool uppercase) {
        if(value < 10) return value + 0x30;
        if(uppercase) return value + 0x41 - 0xa;
        return value + 0x61 - 0xa;
    }

    #define _NUM_BUFFER_SIZE 20

    void __attribute__((format(printf, 4, 5))) Stream::writef(uint32_t flags, void *data, const char *fmt, ...) {
        va_list va;
        va_start(va, fmt);
        this->writef(flags, data, fmt, va);
        va_end(va);
    }

    void __attribute__((format(printf, 4, 0))) Stream::writef(uint32_t flags, void *data, const char *fmt, va_list ap) {
        size_t p = 0;
        int written = 0;
        unsigned long long val;
        int length;
        int base;
        uint32_t tmp;
        int *slot;
        uint8_t *str;

        for(p = 0; fmt[p]; p++) {
            char cur = fmt[p];

            if(cur == '%') {
                length = 0;
                start: {
                    cur = fmt[++p];
                    switch(cur) {
                        case '%':
                            this->write("%", 1, flags, data, &tmp);
                            written ++;
                            break;
                        case 'n':
                            (void)0;
                            slot = va_arg(ap, int *);
                            *slot = written;
                            break;
                        case 'l':
                            length ++;
                            if(length > 2) length = 2;
                            goto start;
                        case 'p':
                            this->write("0x", 2, flags, data, &tmp);
                            written += 2;
                        case 'x':
                        case 'X':
                        case 'o':
                        case 'u':
                        case 'd':
                        case 'i':
                            val = 0;
                            if(!length) val = va_arg(ap, unsigned int);
                            if(length == 1) val = va_arg(ap, unsigned long);
                            if(length == 2) val = va_arg(ap, unsigned long long);
                            base = 10;

                            if(cur == 'x' || cur == 'X' || cur == 'p') base = 16;
                            if(cur == 'o') base = 8;

                            if(cur == 'd' || cur == 'i') {
                                signed int vals = val;
                                if(vals < 0) {
                                    val = -vals;
                                    this->write("-", 1, flags, data, &tmp);
                                    written ++;
                                }
                            }

                            {
                                int start = 0;
                                uint8_t output[_NUM_BUFFER_SIZE];
                                for(int i = _NUM_BUFFER_SIZE - 1; i >= 0; i --) {
                                    output[i] = _num_to_str(val % base, cur == 'X');
                                    val = val / base;
                                }

                                while(output[start] == '0' && start < _NUM_BUFFER_SIZE - 1) start ++;

                                this->write(output + start, _NUM_BUFFER_SIZE - start, flags, data, &tmp);
                                written += _NUM_BUFFER_SIZE - start;
                            }
                            break;
                        case 's':
                            (void)0;
                            str = va_arg(ap, uint8_t *);
                            this->write(str, strlen((char*)str), flags, data, &tmp);
                            written += strlen((char*)str);
                            break;
                    }
                }
            }else{
                this->write(&cur, 1, flags, data, &tmp);
                written ++;
            }
        }

        va_end(ap);
    }

    #undef _NUM_BUFFER_SIZE
}

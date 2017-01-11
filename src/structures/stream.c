#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include "structures/stream.h"
#include "main/errno.h"

/** @todo There are race conditions in the errno thing. Obviously, it can't work as described because what if an
 * Interrupt does stream IO, what would the errno be?
 *
 * @todo stream_writef Doesn't handle errors in the stream, or writing less than it expected to.
 */

int stream_write(stream_t *stream, void *buff, size_t len, uint32_t flags, void *data) {
    if(!stream->write) {
        stream->errno = EPERM;
        return 0;
    }
    
    return stream->write(buff, len, flags, data, stream);
}

int stream_read(stream_t *stream, void *buff, size_t len, uint32_t flags, void *data) {
    if(!stream->read) {
        stream->errno = EPERM;
        return 0;
    }
    
    return stream->read(buff, len, flags, data, stream);
}

int stream_peek(stream_t *stream, void *buff, size_t len, uint32_t flags, void *data) {
    if(!stream->peek) {
        stream->errno = EPERM;
        return 0;
    }
    
    return stream->peek(buff, len, flags, data, stream);
}

int stream_skip(stream_t *stream, size_t len, uint32_t flags, void *data) {
    if(!stream->skip) {
        stream->errno = EPERM;
        return 0;
    }
    
    return stream->skip(len, flags, data, stream);
}

void stream_clear_error(stream_t *stream) {
    stream->errno = 0;
}

static int _strlen(uint8_t *str) {
    int len = 0;
    while(str[len]) len ++;
    return len;
}

static uint8_t _num_to_str(uint8_t value, bool uppercase) {
    if(value < 10) return value + 0x30;
    if(uppercase) return value + 0x41 - 0xa;
    return value + 0x61 - 0xa;
}

#define _NUM_BUFFER_SIZE 20

void stream_writef(stream_t *stream, uint32_t flags, void *data, char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vstream_writef(stream, flags, data, fmt, va);
    va_end(va);
}

void vstream_writef(stream_t *stream, uint32_t flags, void *data, char *fmt, va_list ap) {
    size_t p = 0;
    int written = 0;
    unsigned long long val;
    int length;
    int base;
    
    for(p = 0; fmt[p]; p++) {
        char cur = fmt[p];
        
        if(cur == '%') {
            length = 0;
            start: {
                cur = fmt[++p];
                switch(cur) {
                    case '%':
                        stream_write(stream, "%", 1, flags, data);
                        written ++;
                        break;
                    case 'n':
                        (void)0;
                        int *slot = va_arg(ap, int *);
                        *slot = written;
                        break;
                    case 'l':
                        length ++;
                        if(length > 2) length = 2;
                        goto start;
                    case 'p':
                        stream_write(stream, "0x", 2, flags, data);
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
                                stream_write(stream, "-", 1, flags, data);
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
                            
                            stream_write(stream, output + start, _NUM_BUFFER_SIZE - start, flags, data);
                            written += _NUM_BUFFER_SIZE - start;
                        }
                        break;
                    case 's':
                        (void)0;
                        uint8_t *str = va_arg(ap, uint8_t *);
                        stream_write(stream, str, _strlen(str), flags, data);
                        written += _strlen(str);
                        break;
                }
            }
        }else{
            stream_write(stream, &cur, 1, flags, data);
            written ++;
        }
    }
    
    va_end(ap);
}

#undef _NUM_BUFFER_SIZE

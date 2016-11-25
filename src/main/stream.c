#include <stdint.h>

#include "main/stream.h"
#include "main/errno.h"

int stream_write(stream_t *stream, void *buff, size_t *len, uint32_t flags, void *data) {
    if(!stream->write) {
        stream->errno = EPERM;
        return 0;
    }
    
    return stream->write(buff, len, flags, data);
}

int stream_read(stream_t *stream, void *buff, size_t *len, uint32_t flags, void *data) {
    if(!stream->read) {
        stream->errno = EPERM;
        return 0;
    }
    
    return stream->read(buff, len, flags, data);
}

int stream_peek(stream_t *stream, void *buff, size_t *len, uint32_t flags, void *data) {
    if(!stream->peek) {
        stream->errno = EPERM;
        return 0;
    }
    
    return stream->peek(buff, len, flags, data);
}

int stream_skip(stream_t *stream, size_t *len, uint32_t flags, void *data) {
    if(!stream->skip) {
        stream->errno = EPERM;
        return 0;
    }
    
    return stream->skip(len, flags, data);
}

void stream_clear_error(stream_t *stream) {
    stream->errno = 0;
}

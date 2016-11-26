#ifndef _H_MAIN_STREAM_
#define _H_MAIN_STREAM_

#include <stdint.h>
#include <stddef.h>

typedef int (*stream_write_t)(void *buff, size_t len, uint32_t flags, void *data);
typedef int (*stream_read_t)(void *buff, size_t len, uint32_t flags, void *data);
typedef int (*stream_peek_t)(void *buff, size_t len, uint32_t flags, void *data);
typedef int (*stream_skip_t)(size_t len, uint32_t flags, void *data);

typedef struct stream_s {
    stream_write_t write;
    stream_read_t read;
    stream_peek_t peek;
    stream_skip_t skip;
    int errno;
} stream_t;

int stream_write(stream_t *stream, void *buff, size_t len, uint32_t flags, void *data);
int stream_read(stream_t *stream, void *buff, size_t len, uint32_t flags, void *data);
int stream_peek(stream_t *stream, void *buff, size_t len, uint32_t flags, void *data);
int stream_skip(stream_t *stream, size_t len, uint32_t flags, void *data);

void stream_clear_error(stream_t *stream);

void stream_writef(stream_t *stream, uint32_t flags, void *data, char *fmt, ...);

#endif

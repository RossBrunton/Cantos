#ifndef _H_MAIN_STREAM_
#define _H_MAIN_STREAM_

/** @file structures/stream.h
 *
 * A "stream" is an abstraction that allows different parts of the kernel to communicate with each other.
 *
 * Basically, a stream has a destination. The destination is a string of bytes that can be written to or read from.
 *  There is no requirement that a write followed by a read should return the same information. Streams also do not have
 *  a concept of "size". They should be seen as an infinite sequence of bytes.
 *
 * Streams may support the following operations, which have an appropriate member in the stream structure:
 * * `write`: The given bytes will be sent to the stream source.
 * * `read`: Read some number of bytes from the source into the given buffer, which "consumes" them. Further reads
 *  should not get the same information.
 * * `peek`: As read, but the data is not consumed, meaning that a later "read" should get the same data.
 * * `skip`: As read, but the data isn't written to a buffer, instead it is just discarded.
 *
 * The operations also have a `flags` and `data` parameter. The meaning of these are dependant on the stream itself.
 *
 * Not all streams support all operations, if the operation is not supported, then @ref EPERM is set to the stream's
 *  error number. If any other errors happen, the error on the stream should be set to an appropriate value by the
 *  handlers. The stream's errno should be set to 0 by handlers when there is no error.
 *
 * Streams may read or write less than requested, even 0 bytes, without it being considered an error.
 *
 * Streams can be created by filling in the fields of @ref stream_t with handler functions.
 *
 * Stream implementations block until there is data, unless otherwise documented. If a stream is told not to block, and
 *  has no information.
 *
 * This namespace provides functions for performing stream operations, which should be called instead of the struct's
 *  member functions.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

/** A single stream
 *
 * Provides the handler functions, as well as an error number and additonal data. See the documentation at
 *  @ref structures/stream.h for more information on streams.
 */
typedef struct stream_s stream_t;

/** A write function for the stream, to be set in the stream structure.
 *
 * @param[in] buff The buffer to read the data from.
 * @param[in] len The amount of bytes to write.
 * @param[in] flags Stream dependant flags.
 * @param[in] data Stream dependant extra data.
 * @param[in] stream The stream that is being written into.
 * @return The number of bytes written.
 */
typedef int (*stream_write_t)(void *buff, size_t len, uint32_t flags, void *data, stream_t *stream);
/** A read function for the stream, to be set in the stream structure.
 *
 * @param[out] buff The buffer to read the data into.
 * @param[in] len The maximum amount of bytes to read.
 * @param[in] flags Stream dependant flags.
 * @param[in] data Stream dependant extra data.
 * @param[in] stream The stream that is being read from.
 * @return The number of bytes read.
 */
typedef int (*stream_read_t)(void *buff, size_t len, uint32_t flags, void *data, stream_t *stream);
/** A peek function for the stream, to be set in the stream structure.
 *
 * @param[out] buff The buffer to read the data into.
 * @param[in] len The maximum amount of bytes to read.
 * @param[in] flags Stream dependant flags.
 * @param[in] data Stream dependant extra data.
 * @param[in] stream The stream that is being read from.
 * @return The number of bytes read.
 */
typedef int (*stream_peek_t)(void *buff, size_t len, uint32_t flags, void *data, stream_t *stream);
/** A skip function for the stream, to be set in the stream structure.
 *
 * @param[in] len The maximum amount of bytes to skip.
 * @param[in] flags Stream dependant flags.
 * @param[in] data Stream dependant extra data.
 * @param[in] stream The stream that is being read from.
 * @return The number of bytes skipped.
 */
typedef int (*stream_skip_t)(size_t len, uint32_t flags, void *data, stream_t *stream);

struct stream_s {
    stream_write_t write; /**< The function called when writing to the stream */
    stream_read_t read; /**< The function called when reading from the stream */
    stream_peek_t peek; /**< The function called when peeking from the stream */
    stream_skip_t skip; /**< The function called when skipping data in the stream */
    int errno; /**< The last error occuring in the stream */
    void *data; /**< Stream specific information. Handlers can access this using `stream->data` and its value is defined
     by the stream itself. */
};

/** Writes to the given stream.
 *
 * @param[in] stream The stream to write into.
 * @param[in] buff The buffer to read the data from.
 * @param[in] len The maximum amount of bytes to write.
 * @param[in] flags Stream dependant flags.
 * @param[in] data Stream dependant extra data.
 * @return The number of bytes written.
 */
int stream_write(stream_t *stream, void *buff, size_t len, uint32_t flags, void *data);
/** Reads from the given stream.
 *
 * @param[in] stream The stream to read from.
 * @param[out] buff The buffer to read the data into.
 * @param[in] len The maximum amount of bytes to write.
 * @param[in] flags Stream dependant flags.
 * @param[in] data Stream dependant extra data.
 * @return The number of bytes read.
 */
int stream_read(stream_t *stream, void *buff, size_t len, uint32_t flags, void *data);
/** Reads from the given stream, without consuming the bytes.
 *
 * @param[in] stream The stream to read from.
 * @param[out] buff The buffer to read the data into.
 * @param[in] len The maximum amount of bytes to write.
 * @param[in] flags Stream dependant flags.
 * @param[in] data Stream dependant extra data.
 * @return The number of bytes read.
 */
int stream_peek(stream_t *stream, void *buff, size_t len, uint32_t flags, void *data);
/** Discard the given number of bytes from the stream.
 *
 * @param[in] stream The stream to discard from.
 * @param[in] len The maximum amount of bytes to discard.
 * @param[in] flags Stream dependant flags.
 * @param[in] data Stream dependant extra data.
 * @return The number of bytes discarded.
 */
int stream_skip(stream_t *stream, size_t len, uint32_t flags, void *data);

/** Clears the error number in the stream.
 *
 * That is, sets it to 0.
 *
 * @param[in,out] stream The stream to clear the error for.
 */
void stream_clear_error(stream_t *stream);

/** Writes to the given stream using printf semantics.
 *
 * See @ref stream_writef for more information, this is a version that takes a `va_list` rather than parameters.
 *
 * @param[in] stream The stream to write to.
 * @param[in] flags Stream dependant flags.
 * @param[in] data Stream dependant extra data.
 * @param[in] fmt The format string.
 * @param[in] va The argument list from va_start.
 */
void vstream_writef(stream_t *stream, uint32_t flags, void *data, char *fmt, va_list va);
/** Writes to the given stream using printf semantics.
 *
 * Many of the same values supported by printf are also supported here, but there may be some differences.
 *
 * The format string will be printed as is, but "format specifiers", which start with `%` are replaced with the next
 *  argument in the argument list in a given format.
 *
 * The syntax of a format specifier is `%[length]specifier`.
 *
 * Valid specifiers are:
 * * `%`: a literal `%`.
 * * `n`: Nothing, but the respective argument (which must be a `signed int *`) is updated with the number of characters
 *  writen.
 * * `p`: Pointer, same as `x` only with `0x` prepended.
 * * `x`: Unsigned hexadecimal integer, with `abcdef` in lower case.
 * * `X`: Unsigned hexadecimal integer, with `ABCDEF` in upper case.
 * * `o`: Unsigned octal integer.
 * * `u`: Unsigned decimal integer.
 * * `d`/`i`: Signed decimal integer.
 * * `s`: String, the argument must be a char pointer which is then included as a string.
 *
 * Length specifiers are used to determine the length of the value to take, which only make sense on integers:
 * * `(none)`: Int.
 * * `l`: Long int.
 * * `ll`: Long long int.
 *
 * @param[in] stream The stream to write to.
 * @param[in] flags Stream dependant flags.
 * @param[in] data Stream dependant extra data.
 * @param[in] fmt The format string.
 * @param[in] ... The format parameters.
 */
void stream_writef(stream_t *stream, uint32_t flags, void *data, char *fmt, ...);

#endif

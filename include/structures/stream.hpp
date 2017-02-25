#ifndef _H_MAIN_STREAM_
#define _H_MAIN_STREAM_

/** @file structures/stream.hpp
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
 * Not all streams support all operations, if the operation is not supported, then @ref EPERM is returned from the
 *  appropriate function. If any other errors happen, that error will be the return value of the appropriate function,
 *  in this case, you should assume that any read or written data is junk.
 *
 * Streams may read or write less than requested, even 0 bytes, without it being considered an error.
 *
 * Stream implementations block until there is data, unless otherwise documented.
 *
 * This namespace provides a Stream class, which should be used to create and access streams.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "main/errno.h"

namespace stream {
    class Stream {
    public:
        /** Writes to the stream.
         *
         * @param[in] buff The buffer to read the data from.
         * @param[in] len The maximum amount of bytes to write.
         * @param[in] flags Stream dependant flags.
         * @param[in] data Stream dependant extra data.
         * @param[out] written The number of bytes written.
         * @return Any error made while writing.
         */
        virtual error_t write(const void *buff, size_t len, uint32_t flags, void *data, uint32_t *written);
        /** Reads from the stream.
         *
         * @param[out] buff The buffer to read the data into.
         * @param[in] len The maximum amount of bytes to write.
         * @param[in] flags Stream dependant flags.
         * @param[in] data Stream dependant extra data.
         * @param[out] read The number of bytes read.
         * @return Any error made while reading.
         */
        virtual error_t read(void *buff, size_t len, uint32_t flags, void *data, uint32_t *read);
        /** Reads from the given stream, without consuming the bytes.
         *
         * @param[out] buff The buffer to read the data into.
         * @param[in] len The maximum amount of bytes to write.
         * @param[in] flags Stream dependant flags.
         * @param[in] data Stream dependant extra data.
         * @param[out] read The number of bytes read.
         * @return Any error made while reading.
         */
        virtual error_t peek(void *buff, size_t len, uint32_t flags, void *data, uint32_t *read);
        /** Discard the given number of bytes from the stream.
         *
         * @param[in] len The maximum amount of bytes to discard.
         * @param[in] flags Stream dependant flags.
         * @param[in] data Stream dependant extra data.
         * @param[out] skipped The number of bytes skipped.
         * @return Any error made while reading.
         */
        virtual error_t skip(size_t len, uint32_t flags, void *data, uint32_t *skipped);
        
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
        void __attribute__((format(printf, 4, 0))) vwritef(uint32_t flags, void *data, char *fmt, va_list va);
        
        /** Writes to the given stream using printf semantics.
         *
         * Many of the same values supported by printf are also supported here, but there may be some differences.
         *
         * The format string will be printed as is, but "format specifiers", which start with `%` are replaced with the
         *  next argument in the argument list in a given format.
         *
         * The syntax of a format specifier is `%[length]specifier`.
         *
         * Valid specifiers are:
         * * `%`: a literal `%`.
         * * `n`: Nothing, but the respective argument (which must be a `signed int *`) is updated with the number of
         *  characters written.
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
        void __attribute__((format(printf, 4, 5))) writef(uint32_t flags, void *data, char *fmt, ...);
    };
}

#endif

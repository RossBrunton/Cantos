#ifndef _HPP_STRUCTURES_UTF8_
#define _HPP_STRUCTURES_UTF8_

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "structures/shared_ptr.hpp"

/** Contains a UTF-8 encoded string class */
namespace utf8 {
    /** A string class, similar to std::string mapping UTF-8 encoded text
     *
     * A Utf8 instance contains a null-terminated character string, which is assumed to contain UTF-8 encoded text.
     *  This class provides a number of methods to operate on such strings.
     *
     * Utf8s may or may not own their internal strings, depending on which constructor is called. If the Utf8 does own
     *  its internal buffer, then the buffer will be deleted when no Utf8s own it. In addition, the internal string
     *  buffer (gotten from Utf8::to_string) may become invalid when all of its owning Utf8s are destroyed.
     *
     * Utf8s are immutable; the strings they store should not be modified.
     *
     * The Utf8 class is visible to the global scope.
     */
    class Utf8 {
    public:
        /** Construct a new Utf8 using the given const char buffer
         *
         * This allows creating a Utf8 by doing `Utf8("Some String")`.
         *
         * The Utf8 will not own its internal string, and so it will not be deleted when all Utf8s owning it die. If
         *  you want this behaviour, use Utf8::own instead.
         */
        Utf8(const char *string);
        /** Construct a new Utf8 which is a copy of the given Utf8
         *
         * It will be a reference to the same string as the argument.
         */
        Utf8(const Utf8 &copy);
        /** Construct a new Utf8 by moving an existing Utf8
         *
         * The old Utf8 will be set to `""`.
         */
        Utf8(Utf8 &&move);
        /** Create a new, empty Utf8
         *
         * The string it stores will be `""`.
         */
        Utf8();
        ~Utf8() {};
        /** Create a Utf8 from the given string, but it owns its buffer
         *
         * Use this, for example, when you have malloced a buffer, and want it to automatically be freed when no longer
         *  needed.
         *
         * @param buffer The string to give to the Utf8
         * @param length The length of the string given to the Utf8 (excluding the null terminator), if this is npos,
         *  then the string length will be calculated instead.
         * @return A Utf8 owning the given string
         */
        static Utf8 own(const char *buffer, size_t length = npos);

        /** Return the internal buffer, as a conventional C string
         *
         * It will be null terminated.
         *
         * @return This Utf8 as a traditional C string
         */
        const char *to_string() const;

        /** Returns the number of bytes in the string
         *
         * This does not include the terminating null character. This value is cached, and so this function runs in
         *  O(1) time.
         *
         * @return The number of bytes in the string
         */
        uint32_t bytes() const;
        /** Returns the number of logical UTF-8 characters in the string
         *
         * This is calculated on the spot, in O(n) time.
         *
         * @return The number of characters in the string
         */
        uint32_t chars() const;
        /** Return a substring starting from pos of len bytes
         *
         * If the length would result in the substring overruning the end of the source string, the rest of the string
         *  is taken instead.
         *
         * This runs in O(n) time.
         *
         * @param pos The position to start the substring at
         * @param len The number of bytes in the substring
         * @return A new Utf8 that is a substring of the source string
         */
        Utf8 substr(size_t pos, size_t len) const;

        /** Returns whether this string is empty (i.e. bytes() == 0)
         *
         * @return Whether the string is empty
         */
        bool empty() {
            return byte_count == 0;
        }

        /** Compare this Utf8 with another
         *
         * Compare the two Utf8s and return a value as appropriate:
         * * 0: Both strings are equal.
         * * <0: The value of the first non-matching character is lower in str, or all characters match but str is
         *  shorter.
         * * >0: The value of the first non-matching character is greater in str, or all characters match but str is
         *  longer.
         *
         * All compare functions run in O(n) time.
         *
         * @param str The string to compare against
         * @return A result as described above
         */
        int compare(const Utf8& str) const;
        /** Compare another string with a substring of this Utf8
         *
         * See compare(const Utf8& str), but compares against a substring of this string, rather than the whole string.
         *
         * @param pos The position to start the substring of this at
         * @param len The length of the substring of this
         * @param str The string to compare against
         * @return A result as described in Utf8::compare(const Utf8& str)
         */
        int compare(size_t pos, size_t len, const Utf8& str) const;
        /** Compare a substring of another string with a substring of this Utf8
         *
         * See compare(const Utf8& str), but compares a substring of this against a substring of str, rather than using
         *  whole strings.
         *
         * @param pos The position to start the substring of this at
         * @param len The length of the substring of this
         * @param str The string to take a substring out of
         * @param subpos The position to start the substring of str at
         * @param sublen The length of the substring of str
         * @return A result as described in Utf8::compare(const Utf8& str)
         */
        int compare(size_t pos, size_t len, const Utf8& str, size_t subpos, size_t sublen) const;

        /** Finds a substring in this string
         *
         * Returns Utf8::npos if it is not found, otherwise returns the index of the first character found.
         *
         * This function runs in O(n) time.
         *
         * @param str The substring to search for
         * @param pos The position to start searching at
         * @return The first index the substring was found at, or Utf8::npos if it wasn't found
         */
        size_t find(const Utf8 &str, size_t pos = 0) const;
        /** Finds a given character in this string
         *
         * Returns Utf8::npos if it is not found, otherwise returns the index of the first occurence of this character.
         *
         * This function runs in O(n) time.
         *
         * @param c The character to search for
         * @pos The position in the string to start searching at
         * @return The index of the first occurence of the character.
         */
        size_t find(char c, size_t pos = 0) const;

        /** Copy the string from the other Utf8 into this one
         *
         * After this call, this Utf8 will store the string that the other Utf8 had.
         *
         * Assigning a string runs in O(1) time.
         */
        Utf8& operator=(Utf8& other);
        /** Move the string from the other Utf8 into this one
         *
         * After this call, this Utf8 will store the string owned by the other one, and the other Utf8 will store `""`.
         *
         * Assigning a string runs in O(1) time.
         */
        Utf8& operator=(Utf8&& other);
        /** Compare this Utf8 with another
         *
         * Two Utf8s are equal iff they are the same length, and all bytes inside the strings are equal.
         *
         * Comparing a string runs in O(n) time.
         */
        bool operator==(const Utf8& other) const;
        /** Compare this Utf8 with another
         *
         * Two Utf8s are not equal iff they are different lengths, or any characters inside the strings differ.
         *
         * Comparing a string runs in O(n) time.
         */
        bool operator!=(const Utf8& other) const;
        /** Compare this Utf8 with a null terminated string
         *
         * It will be equal to the string iff they are the same length, and all the characters inside the string match.
         *
         * Comparing a string runs in O(n) time.
         */
        bool operator==(const char *other) const;
        /** Compare this Utf8 with a null terminated string
         *
         * They will not be equal iff they are different lengths, or any of the characters inside the strings differ.
         *
         * Comparing a string runs in O(n) time.
         */
        bool operator!=(const char *other) const;
        /** Concatenate two Utf8s together
         *
         * The resulting string will be a new string with the contents of this string (minus the null terminator),
         *  followed by the other string.
         *
         * This runs in O(n) time.
         */
        Utf8 operator+(const Utf8& other);
        /** Concatenate this Utf8 and a null terminated string
         *
         * The resulting string will be a new string with the contents of this string (minus the null terminator),
         *  followed by the other string.
         *
         * This runs in O(n) time.
         */
        Utf8 operator+(const char *other);
        /** Access a specific character of the underlying string
         *
         * This runs in O(1) time.
         */
        char operator[] (int x) const;

        /** Performs a sprintf-like format with this as the format parameter
         *
         * This returns a new version of the current Utf8, but with format specifiers (starting with '%') being
         *  replaced by appropriate values given in the arguments to this call.
         *
         * Many of the same values supported by printf are also supported here, but there may be some differences.
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
         * * `S`: Utf8*, a pointer to a Uft8 instance to include
         *
         * Length specifiers are used to determine the length of the value to take, which only make sense on integers:
         * * `(none)`: Int.
         * * `l`: Long int.
         * * `ll`: Long long int.
         *
         * @param ... The format parameters.
         */
        Utf8 format(...) const;
        /** Performs a sprintf-like format with this as the format parameter
         *
         * See @ref format for more information, this is a version that takes a `va_list` rather than parameters.
         *
         * @param ap The argument list from va_start.
         */
        Utf8 format(va_list ap) const;

        /** Maximum value for the length of a string
         *
         * This is returned by various functions to indicate several conditions. For example, a substring was not found
         *  or a substring of "the rest of the string"
         */
        static const size_t npos = -1;

    private:
        shared_ptr_ns::shared_ptr<const char> string_ptr;
        const char *string;
        size_t byte_count = 0;
    };
}

#endif

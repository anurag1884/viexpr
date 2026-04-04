#ifndef STR_H
#define STR_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>

    // Structure that holds arrays of characters (strings)
    typedef struct str
    {
        size_t len, cap;
        char *data;
    } str_t;

    // Creates a string with a given length and capacity
    str_t str_create(size_t len, size_t cap);

    // Creates a string from a single character
    str_t str_fromchar(char ch);

    // Creates a copy over an immutable C-string (safe to destroy)
    str_t str_ccopy(const char *cs);

    // Creates a copy from another string object (safe to destroy)
    str_t str_copy(const str_t *s);

    // Formatting support for the string class
    str_t str_format(const char *fmt, ...);

    // Appends a string at the end of the given string
    void str_append(str_t *s, const str_t *a);

    // Null terminates the string at the end (useful for use in string functions
    // from the standard library)
    void str_terminate(str_t *s);

    // Pushes a character to the end of the string
    void str_push_back(str_t *s, char ch);

    // Pops out the last character from the string (Does not account for the NUL
    // terminator)
    char str_pop_back(str_t *s);

    // Sets the character at the specified index in the string
    void str_set_at(str_t *s, size_t p, char ch);

    // Retrieves the element at the specified index in the string
    char str_at(const str_t *s, size_t p);

    // Retrieves the last character in the string
    char str_top(const str_t *s);

    // Clears out the string by setting its length to zero
    void str_clear(str_t *s);

    // Destroys a given string
    void str_destroy(str_t *s);

#ifdef __cplusplus
}
#endif

#endif /* STR_H */

#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "entry.h"
#include "str.h"
#include "vec.h"

#include <stdbool.h>

// Initial capacity to use in containers
#define INITIAL_CAP 16

    // Finds the next power of 2 using bit-smearing
    size_t next_pow2(size_t n);

    // Checks whether the given string is a valid ID or not
    bool is_valid_id(const str_t *s);

    // Returns whether the given string is all whitespace or not
    bool is_all_spaces(const str_t *s);

    // Returns whether the given token is a valid floating-point number
    bool is_valid_float(const str_t *token);

    // Finds the indices of the expression ID, expression and expression type
    // columns by searching through the header.
    void find_cols(const vec_t *tokens, size_t *id_col, size_t *expr_col,
                   size_t *type_col);

    // Gets the expression type enumerator value corresponding to the string
    expr_type_t get_expression_type(const str_t *s);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_H */

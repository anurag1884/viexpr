#ifndef TOKEN_H
#define TOKEN_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "str.h"
#include "vec.h"

#include <stdbool.h>

    // 0: operator, 1: operand
    typedef enum token_type
    {
        type_operator,
        type_operand
    } token_type_t;

    // Structure that holds a token of a specified type (either an operator or
    // an operand)
    typedef struct token
    {
        token_type_t type;
        str_t token;
    } token_t;

    // Creates a token with a specified type
    token_t token_create(token_type_t type, const str_t *token);

    // Creates a copy of the given token
    token_t token_copy(const token_t *t);

    // Converts a given expression to a vector of tokens
    vec_t expr_to_tokens(const str_t *expr);

    // Converts a vector of tokens to a concatenated string expression
    str_t tokens_to_expr(const vec_t *tokens);

    // Destroys a given token
    void token_destroy(token_t *t);

#ifdef __cplusplus
}
#endif

#endif /* TOKEN_H */

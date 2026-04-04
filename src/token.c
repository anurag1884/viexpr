#include "token.h"
#include "utils.h"

#include <ctype.h>

token_t token_create(token_type_t type, const str_t *token)
{
    token_t t;

    t.type = type;
    t.token = str_copy(token);

    str_terminate(&t.token);

    return t;
}

token_t token_copy(const token_t *t)
{
    return token_create(t->type, &t->token);
}

vec_t expr_to_tokens(const str_t *expr)
{
    char ch, last_ch = '\0';

    size_t i;

    str_t token = str_create(0, INITIAL_CAP);

    token_t t;

    vec_t tokens = vec_create(0, INITIAL_CAP, token_t);

    for (i = 0; i < expr->len; i++)
    {
        ch = str_at(expr, i);

        if (isalnum(ch) || ch == '.' ||
            (last_ch == 'e' && (ch == '+' || ch == '-')))
            str_push_back(&token, ch);
        else
        {
            if (token.len != 0)
            {
                str_terminate(&token);
                t = token_create(type_operand, &token);
                vec_push_back(&tokens, &t);
                str_clear(&token);
            }
            if (!isspace(ch))
            {
                str_push_back(&token, ch);
                str_terminate(&token);
                t = token_create(type_operator, &token);
                vec_push_back(&tokens, &t);
                str_clear(&token);
            }
        }

        last_ch = ch;
    }

    if (token.len != 0)
    {
        str_terminate(&token);
        t = token_create(type_operand, &token);
        vec_push_back(&tokens, &t);
    }

    str_destroy(&token);

    return tokens;
}

str_t tokens_to_expr(const vec_t *tokens)
{
    size_t i;

    str_t res = str_create(0, INITIAL_CAP), current_token;

    if (tokens->len != 0)
    {
        for (i = 0; i < tokens->len - 1; i++)
        {
            current_token = ((token_t *)vec_at(tokens, i))->token;
            str_append(&res, &current_token);
            str_push_back(&res, ' ');
        }
        current_token = ((token_t *)vec_at(tokens, tokens->len - 1))->token;
        str_append(&res, &current_token);
    }

    str_terminate(&res);

    return res;
}

void token_destroy(token_t *token)
{
    str_destroy(&token->token);
}

#include "utils.h"

#include <ctype.h>
#include <limits.h>
#include <string.h>

size_t next_pow2(size_t n)
{
    if (n <= 1)
        return 1;

    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;

    return n + 1;
}

bool is_valid_id(const str_t *s)
{
    bool digit_flag = false, number_flag = false;

    size_t i;

    for (i = 0; i < s->len; i++)
    {
        char ch = str_at(s, i);

        if (isspace(ch))
        {
            if (digit_flag)
                // Set number count to 1 after first number
                number_flag = true;
        }
        else
        {
            if (isdigit(ch))
            {
                if (number_flag)
                    // In case another number is spotted after whitespace, e.g.,
                    // "12 34", consider as invalid
                    return false;
                digit_flag = true;
            }
            else
                return false;
        }
    }
    return digit_flag;
}

bool is_all_spaces(const str_t *s)
{
    size_t i;

    for (i = 0; i < s->len; i++)
        if (!isspace(str_at(s, i)))
            return false;
    return true;
}

bool is_valid_float(const str_t *token)
{
    char ch, last_ch = '\0';

    size_t i;

    for (i = 0; i < token->len; i++)
    {
        ch = str_at(token, i);

        if (isalpha(ch) && !((isdigit(last_ch) || last_ch == '.') && ch == 'e'))
            return false;

        last_ch = ch;
    }
    return true;
}

void find_cols(const vec_t *tokens, size_t *id_col, size_t *expr_col,
               size_t *type_col)
{
    size_t i;

    *id_col = *expr_col = *type_col = SIZE_MAX;
    for (i = 0; i < tokens->len; i++)
    {
        str_t *it = ((str_t *)vec_at(tokens, i));

        if (*id_col != SIZE_MAX && *expr_col != SIZE_MAX &&
            *type_col != SIZE_MAX)
            break;
        if (*id_col == SIZE_MAX && strcmp(it->data, "ExprID") == 0)
            *id_col = i;
        if (*expr_col == SIZE_MAX && strcmp(it->data, "Expression") == 0)
            *expr_col = i;
        if (*type_col == SIZE_MAX && strcmp(it->data, "Type") == 0)
            *type_col = i;
    }
}

expr_type_t get_expression_type(const str_t *s)
{
    size_t i;

    for (i = 0; i < s->len; i++)
        if (!isspace(str_at(s, i)))
            break;

    if (strcmp(s->data + i, "Infix") == 0)
        return type_infix;
    else if (strcmp(s->data + i, "Postfix") == 0)
        return type_postfix;
    else
        return type_unknown;
}

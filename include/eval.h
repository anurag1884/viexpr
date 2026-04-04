#ifndef EVAL_H
#define EVAL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "str.h"
#include "vec.h"

#include <stdbool.h>

    // Converts a given infix expression to postfix
    str_t infix_to_postfix(const vec_t *infix, vec_t *postfix,
                           vec_t *infix_op_stack_hist, vec_t *postfix_hist);

    // Evaluates a given postfix expression and yields the result
    str_t postfix_evaluate(const vec_t *postfix, double *res,
                           str_t *postfix_op_hist, vec_t *number_stack_hist);

    // Checks whether the provided infix expression is valid or not
    bool is_valid_infix(const vec_t *infix);

    // Checks whether the provided postfix expression is valid or not
    bool is_valid_postfix(const vec_t *postfix);

#ifdef __cplusplus
}
#endif

#endif /* EVAL_H */

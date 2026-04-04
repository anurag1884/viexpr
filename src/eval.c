#include "eval.h"
#include "nums.h"
#include "token.h"
#include "utils.h"

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

size_t priority(char op)
{
    switch (op)
    {
    // Unary negation operator
    case '~':
        return 5;

    // Exponent operator
    case '^':
        return 4;

    // Multiplication and Division operators
    case '*':
    case '/':
        return 3;

    // Addition and Subtraction operators
    case '+':
    case '-':
        return 2;

    // Left parethesis considered to be lowest priority
    case '(':
        return 1;

    // Invalid operators are assigned '0' priority
    default:
        return 0;
    }
}

void add_infix_op_stack_history(vec_t *hist, str_t *stack)
{
    str_t copy = str_copy(stack);

    str_terminate(&copy);
    vec_push_back(hist, &copy);
}

void add_postfix_history(vec_t *hist, vec_t *postfix)
{
    vec_t copy = vec_create(0, postfix->len, token_t);

    token_t t_copy;

    size_t i;

    for (i = 0; i < postfix->len; i++)
    {
        t_copy = token_copy((token_t *)vec_at(postfix, i));

        vec_push_back(&copy, &t_copy);
    }
    vec_push_back(hist, &copy);
}

bool left_associative_priority_cmp(char op1, char op2)
{
    return priority(op1) >= priority(op2);
}

bool right_associative_priority_cmp(char op1, char op2)
{
    return priority(op1) > priority(op2);
}

str_t infix_to_postfix(const vec_t *infix, vec_t *postfix,
                       vec_t *infix_op_stack_hist, vec_t *postfix_hist)
{
    char op, current_op;

    token_type_t prev_state = type_operator;

    size_t l_paren_count = 0, r_paren_count = 0, i;

    str_t stack = str_create(0, INITIAL_CAP), message = str_create(0, 0),
          infix_expr;

    token_t *current_token, t;

    bool (*priority_cmp)(char op1, char op2);

    for (i = 0; i < infix->len; i++)
    {
        current_token = (token_t *)vec_at(infix, i);

        if (current_token->type == type_operator)
        {
            current_op = current_token->token.data[0];

            switch (current_op)
            {
            // Open parenthesis
            case '(':
                if (prev_state == type_operand)
                {
                    infix_expr = tokens_to_expr(infix);
                    message = str_format(
                        "Unexpected open parenthesis encountered in an incomplete infix expression: '%s'.",
                        infix_expr.data);
                    str_destroy(&infix_expr);

                    str_destroy(&stack);

                    return message;
                }
                l_paren_count++;

                str_push_back(&stack, current_op);

                add_infix_op_stack_history(infix_op_stack_hist, &stack);
                add_postfix_history(postfix_hist, postfix);
                break;

            // Close parenthesis
            case ')':
                if (prev_state == type_operator)
                {
                    infix_expr = tokens_to_expr(infix);
                    message = str_format(
                        "Unexpected close parenthesis encountered in an incomplete infix expression: '%s'.",
                        infix_expr.data);
                    str_destroy(&infix_expr);

                    str_destroy(&stack);

                    return message;
                }
                r_paren_count++;

                while (stack.len != 0 && (op = str_pop_back(&stack)) &&
                       op != '(')
                {
                    t = (token_t){type_operator, str_fromchar(op)};

                    vec_push_back(postfix, &t);
                }

                add_infix_op_stack_history(infix_op_stack_hist, &stack);
                add_postfix_history(postfix_hist, postfix);
                break;

            // Left associativity
            // Unary/Binary Operators
            case '+':
            case '-':
                if (prev_state == type_operator)
                {
                    if (current_op == '-')
                        str_push_back(&stack, '~');

                    add_infix_op_stack_history(infix_op_stack_hist, &stack);
                    add_postfix_history(postfix_hist, postfix);
                    break;
                }
            // Binary Operators
            case '*':
            case '/':
            // Right associativity
            case '^':
                if (prev_state == type_operator)
                {
                    infix_expr = tokens_to_expr(infix);
                    message = str_format(
                        "Unexpected location of operator '%c' in infix expression: '%s'.",
                        current_op, infix_expr.data);
                    str_destroy(&infix_expr);

                    str_destroy(&stack);

                    return message;
                }

                prev_state = type_operator;

                priority_cmp = current_op == '^'
                                   ? right_associative_priority_cmp
                                   : left_associative_priority_cmp;

                while (stack.len != 0 &&
                       priority_cmp(str_top(&stack), current_op))
                {
                    t = (token_t){type_operator,
                                  str_fromchar(str_pop_back(&stack))};

                    vec_push_back(postfix, &t);
                }
                str_push_back(&stack, current_op);

                add_infix_op_stack_history(infix_op_stack_hist, &stack);
                add_postfix_history(postfix_hist, postfix);
                break;
            default:
                infix_expr = tokens_to_expr(infix);
                message = str_format(
                    "Unsupported operator '%c' in infix expression: '%s'.",
                    current_op, infix_expr.data);
                str_destroy(&infix_expr);

                str_destroy(&stack);

                return message;
            }
        }
        else
        {
            // Check if literal is a valid operand, if not, error
            if (!is_valid_float(&current_token->token))
            {
                infix_expr = tokens_to_expr(infix);
                message = str_format(
                    "Invalid literal '%s' (non-numeric) encountered in infix expression: '%s'.",
                    current_token->token.data, infix_expr.data);
                str_destroy(&infix_expr);

                str_destroy(&stack);

                return message;
            }
            if (prev_state == type_operand)
            {
                infix_expr = tokens_to_expr(infix);
                message = str_format(
                    "Operands encountered without an operator in infix expression: '%s'.",
                    infix_expr.data);
                str_destroy(&infix_expr);

                str_destroy(&stack);

                return message;
            }
            prev_state = type_operand;

            t = token_copy(current_token);

            vec_push_back(postfix, &t);

            add_infix_op_stack_history(infix_op_stack_hist, &stack);
            add_postfix_history(postfix_hist, postfix);
        }
    }

    if (prev_state == type_operator)
    {
        infix_expr = tokens_to_expr(infix);
        message =
            str_format("Incomplete infix expression: '%s'.", infix_expr.data);
        str_destroy(&infix_expr);

        str_destroy(&stack);

        return message;
    }

    if (l_paren_count != r_paren_count)
    {
        infix_expr = tokens_to_expr(infix);
        message = str_format(
            "Unbalanced parenthesis encountered in infix expression: '%s'.",
            infix_expr.data);
        str_destroy(&infix_expr);

        str_destroy(&stack);

        return message;
    }

    // Flush out any remaining operators
    while (stack.len != 0 && (op = str_pop_back(&stack)) && op != '(')
    {
        t = (token_t){type_operator, str_fromchar(op)};

        vec_push_back(postfix, &t);
    }

    add_infix_op_stack_history(infix_op_stack_hist, &stack);
    add_postfix_history(postfix_hist, postfix);

    str_destroy(&stack);

    return message;
}

void add_postfix_op_history(str_t *postfix_op_hist, char op)
{
    str_push_back(postfix_op_hist, op);
}

void add_number_stack_history(vec_t *hist, nums_t *stack)
{
    nums_t copy = nums_copy(stack);

    vec_push_back(hist, &copy);
}

str_t postfix_evaluate(const vec_t *postfix, double *res,
                       str_t *postfix_op_hist, vec_t *number_stack_hist)
{
    char current_op;

    size_t i;

    double a, b;

    nums_t stack = nums_create(0, INITIAL_CAP);

    str_t message = str_create(0, 0), postfix_expr;

    token_t *current_token;

    for (i = 0; i < postfix->len; i++)
    {
        current_token = (token_t *)vec_at(postfix, i);

        if (current_token->type == type_operator)
        {
            current_op = current_token->token.data[0];

            // Pop two operands for the binary operation
            if (stack.len == 0)
            {
                postfix_expr = tokens_to_expr(postfix);
                message = str_format(
                    "Missing operands for operator '%c' in postfix expression: '%s'.",
                    current_op, postfix_expr.data);
                str_destroy(&postfix_expr);

                str_terminate(postfix_op_hist);
                nums_destroy(&stack);

                return message;
            }
            b = nums_pop_back(&stack);
            if (current_op == '~')
                nums_push_back(&stack, -b);
            else if (current_op == '+' || current_op == '-' ||
                     current_op == '*' || current_op == '/' ||
                     current_op == '^')
            {
                if (stack.len == 0)
                {
                    postfix_expr = tokens_to_expr(postfix);
                    message = str_format(
                        "Missing operands for operator '%c' in postfix expression: '%s'.",
                        current_op, postfix_expr.data);
                    str_destroy(&postfix_expr);

                    str_terminate(postfix_op_hist);
                    nums_destroy(&stack);

                    return message;
                }
                a = nums_pop_back(&stack);
                // Perform binary operation depending on the operator
                if (current_op == '+')
                    nums_push_back(&stack, a + b);
                else if (current_op == '-')
                    nums_push_back(&stack, a - b);
                else if (current_op == '*')
                    nums_push_back(&stack, a * b);
                else if (current_op == '/')
                    nums_push_back(&stack, a / b);
                else if (current_op == '^')
                    nums_push_back(&stack, pow(a, b));
            }
            else
            {
                postfix_expr = tokens_to_expr(postfix);
                message = str_format(
                    "Unsupported operator '%c' in postfix expression: '%s'.",
                    current_op, postfix_expr.data);
                str_destroy(&postfix_expr);

                str_terminate(postfix_op_hist);
                nums_destroy(&stack);

                return message;
            }
            add_number_stack_history(number_stack_hist, &stack);
            add_postfix_op_history(postfix_op_hist, current_op);
        }
        else
        {
            // Check if literal is a valid operand, if not, error
            if (!is_valid_float(&current_token->token))
            {
                postfix_expr = tokens_to_expr(postfix);
                message = str_format(
                    "Invalid literal '%s' (non-numeric) encountered in postfix expression: '%s'.",
                    current_token->token.data, postfix_expr.data);
                str_destroy(&postfix_expr);

                str_terminate(postfix_op_hist);
                nums_destroy(&stack);

                return message;
            }
            nums_push_back(&stack,
                           (double)strtod(current_token->token.data, NULL));
            add_number_stack_history(number_stack_hist, &stack);
            add_postfix_op_history(postfix_op_hist, ' ');
        }
    }
    // If only one value is present in the stack, postfix expression is valid
    // and it's our final answer
    if (stack.len == 1)
        *res = nums_pop_back(&stack);
    else
    {
        postfix_expr = tokens_to_expr(postfix);
        message = str_format("Invalid/incomplete postfix expression: '%s'.",
                             postfix_expr.data);
        str_destroy(&postfix_expr);

        str_terminate(postfix_op_hist);
        nums_destroy(&stack);

        return message;
    }

    str_terminate(postfix_op_hist);
    nums_destroy(&stack);

    return message;
}

bool is_valid_infix(const vec_t *infix)
{
    char current_op;

    token_type_t prev_state = type_operator;

    size_t l_paren_count = 0, r_paren_count = 0, i;

    token_t *current_token;

    for (i = 0; i < infix->len; i++)
    {
        current_token = (token_t *)vec_at(infix, i);

        if (current_token->type == type_operator)
        {
            current_op = current_token->token.data[0];

            switch (current_op)
            {
            // Open parenthesis
            case '(':
                if (prev_state == type_operand)
                    return false;
                l_paren_count++;
                break;

            // Close parenthesis
            case ')':
                if (prev_state == type_operator)
                    return false;
                r_paren_count++;
                break;

            // Unary/Binary Operators
            case '+':
            case '-':
                if (prev_state == type_operator)
                    break;
            // Binary Operators
            case '*':
            case '/':
            case '^':
                if (prev_state == type_operator)
                    return false;
                prev_state = type_operator;
                break;
            }
        }
        else
        {
            // Check if literal is a valid operand, if not, error
            if (!is_valid_float(&current_token->token))
                return false;
            if (prev_state == type_operand)
                return false;
            prev_state = type_operand;
        }
    }

    if (prev_state == type_operator)
        return false;

    if (l_paren_count != r_paren_count)
        return false;

    return true;
}

bool is_valid_postfix(const vec_t *postfix)
{
    char current_op;

    size_t i, top = SIZE_MAX;

    token_t *current_token;

    for (i = 0; i < postfix->len; i++)
    {
        current_token = (token_t *)vec_at(postfix, i);

        if (current_token->type == type_operator)
        {
            current_op = current_token->token.data[0];

            switch (current_op)
            {
            // Unary Operators
            case '~':
                // Pop one operand for the unary operation
                if (top == SIZE_MAX)
                    return false;
                break;
            // Binary Operators
            case '+':
            case '-':
            case '*':
            case '/':
            case '^':
                // Pop two operands for the binary operation
                if (top == SIZE_MAX)
                    return false;
                top--;
                if (top == SIZE_MAX)
                    return false;
                break;
            default:
                return false;
            }
        }
        else
        {
            // Check if literal is a valid operand, if not, error
            if (!is_valid_float(&current_token->token))
                return false;
            top++;
        }
    }

    if (top != 0)
        return false;

    return true;
}

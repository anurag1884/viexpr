#ifndef ENTRY_H
#define ENTRY_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "str.h"
#include "vec.h"

#define EXPR_TYPE_TO_STRING(type)                                              \
    ((type) == type_infix     ? "Infix"                                        \
     : (type) == type_postfix ? "Postfix"                                      \
                              : "Unknown")

    // 0: unknown, 1: infix, 2: postfix
    typedef enum expr_type
    {
        type_unknown,
        type_infix,
        type_postfix
    } expr_type_t;

    // Structure that holds data for a given entry
    typedef struct entry
    {
        size_t id;
        expr_type_t type;
        str_t expr;
    } entry_t;

    // Creates an entry (infix/postfix expression)
    entry_t entry_create(size_t id, expr_type_t type, const str_t *expr);

    // Finds a given entry in the entries vector by ExprID
    entry_t *find_entry_by_id(vec_t *entries, size_t id);

    // Destroys a given entry
    void entry_destroy(entry_t *ent);

#ifdef __cplusplus
}
#endif

#endif /* ENTRY_H */

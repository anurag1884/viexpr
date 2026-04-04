#include "entry.h"

entry_t entry_create(size_t id, expr_type_t type, const str_t *expr)
{
    entry_t ent;

    ent.id = id;
    ent.type = type;
    ent.expr = str_copy(expr);

    str_terminate(&ent.expr);

    return ent;
}

entry_t *find_entry_by_id(vec_t *entries, size_t id)
{
    size_t i;

    entry_t *it;

    for (i = 0; i < entries->len; i++)
    {
        it = (entry_t *)vec_at(entries, i);

        if (it->id == id)
            return it;
    }

    return NULL;
}

void entry_destroy(entry_t *ent)
{
    str_destroy(&ent->expr);
}

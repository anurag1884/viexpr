#include "nums.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

nums_t nums_create(size_t len, size_t cap)
{
    nums_t ns;

    ns.len = len;
    ns.cap = cap;
    if (ns.cap == 0)
        ns.data = NULL;
    else
    {
        ns.data = (double *)malloc(ns.cap * sizeof(double));
        if (ns.data == NULL)
            ns.cap = 0;
    }

    return ns;
}

nums_t nums_copy(const nums_t *ns)
{
    nums_t copy = nums_create(ns->len, ns->len);

    memcpy(copy.data, ns->data, ns->len * sizeof(double));

    return copy;
}

void nums_push_back(nums_t *ns, double x)
{
    size_t new_cap;

    double *new_data;

    if (ns->data == NULL || ns->len >= ns->cap)
    {
        new_cap = next_pow2(ns->len + 1);
        new_data = (double *)realloc(ns->data, new_cap * sizeof(double));
        if (new_data == NULL)
            return;
        else
        {
            ns->cap = new_cap;
            ns->data = new_data;
        }
    }
    nums_set_at(ns, ns->len++, x);
}

double nums_pop_back(nums_t *ns)
{
    return nums_at(ns, --ns->len);
}

void nums_set_at(nums_t *ns, size_t p, double x)
{
    ns->data[p] = x;
}

double nums_at(const nums_t *ns, size_t p)
{
    return ns->data[p];
}

double nums_top(const nums_t *ns)
{
    return nums_at(ns, ns->len - 1);
}

void nums_clear(nums_t *ns)
{
    ns->len = 0;
}

void nums_destroy(nums_t *ns)
{
    free(ns->data);
    ns->data = NULL;
    ns->len = ns->cap = 0;
}

#include "vec.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

vec_t vec_create_impl(size_t len, size_t cap, size_t byte_size)
{
    vec_t v;

    v.len = len;
    v.cap = cap;
    v.byte_size = byte_size;

    if (v.cap == 0)
        v.data = NULL;
    else
    {
        v.data = malloc(v.cap * v.byte_size);
        if (v.data == NULL)
            v.cap = 0;
    }

    return v;
}

void vec_push_back(vec_t *v, const void *elem)
{
    size_t new_cap;

    void *new_data;

    if (v->data == NULL || v->len >= v->cap)
    {
        new_cap = next_pow2(v->len + 1);
        new_data = realloc(v->data, new_cap * v->byte_size);
        if (new_data == NULL)
            return;
        else
        {
            v->cap = new_cap;
            v->data = new_data;
        }
    }
    vec_set_at(v, v->len++, elem);
}

void *vec_pop_back(vec_t *v)
{
    return vec_at(v, --v->len);
}

void vec_set_at(vec_t *v, size_t p, const void *elem)
{
    memcpy(vec_at(v, p), elem, v->byte_size);
}

void *vec_at(const vec_t *v, size_t p)
{
    return (char *)v->data + p * v->byte_size;
}

void *vec_top(const vec_t *v)
{
    return vec_at(v, v->len - 1);
}

void vec_clear(vec_t *v)
{
    v->len = 0;
}

void vec_destroy(vec_t *v)
{
    free(v->data);
    v->data = NULL;
    v->len = v->cap = 0;
}

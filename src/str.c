#include "str.h"
#include "utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void expand_str(str_t *s, size_t new_len)
{
    size_t new_cap;

    char *new_data;

    if (s->data == NULL || new_len >= s->cap)
    {
        new_cap = next_pow2(new_len);
        new_data = (char *)realloc(s->data, new_cap);
        if (new_data == NULL)
            return;
        else
        {
            s->cap = new_cap;
            s->data = new_data;
        }
    }
}

str_t str_create(size_t len, size_t cap)
{
    str_t s;

    s.len = len;
    s.cap = cap;
    if (s.cap == 0)
        s.data = NULL;
    else
    {
        s.data = (char *)malloc(s.cap);
        if (s.data == NULL)
            s.cap = 0;
    }

    return s;
}

str_t str_fromchar(char ch)
{
    str_t s = str_create(0, 1);

    str_push_back(&s, ch);

    return s;
}

str_t str_ccopy(const char *cs)
{
    size_t len = strlen(cs);

    str_t s = str_create(len, len + 1);

    memcpy(s.data, cs, len + 1);

    return s;
}

str_t str_copy(const str_t *s)
{
    str_t copy = str_create(s->len, s->len + 1);

    memcpy(copy.data, s->data, s->len);

    return copy;
}

str_t str_format(const char *fmt, ...)
{
    va_list args;

    size_t len;

    str_t message;

    va_start(args, fmt);
    len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    message = str_create(len, len + 1);

    va_start(args, fmt);
    vsnprintf(message.data, len + 1, fmt, args);
    va_end(args);

    return message;
}

void str_append(str_t *s, const str_t *a)
{
    expand_str(s, s->len + a->len);
    memcpy(s->data + s->len, a->data, a->len);
    s->len += a->len;
}

void str_terminate(str_t *s)
{
    expand_str(s, s->len + 1);
    str_set_at(s, s->len, '\0');
}

void str_push_back(str_t *s, char ch)
{
    expand_str(s, s->len + 1);
    str_set_at(s, s->len++, ch);
}

char str_pop_back(str_t *s)
{
    return str_at(s, --s->len);
}

void str_set_at(str_t *s, size_t p, char ch)
{
    s->data[p] = ch;
}

char str_at(const str_t *s, size_t p)
{
    return s->data[p];
}

char str_top(const str_t *s)
{
    return str_at(s, s->len - 1);
}

void str_clear(str_t *s)
{
    s->len = 0;
}

void str_destroy(str_t *s)
{
    free(s->data);
    s->data = NULL;
    s->len = s->cap = 0;
}

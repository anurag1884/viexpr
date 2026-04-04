#ifndef VEC_H
#define VEC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>

// Creates a vector with a given length, capacity and type
#define vec_create(len, cap, type) vec_create_impl((len), (cap), sizeof(type))

    // Structure that holds arrays of elements of an arbitrary type
    typedef struct vec
    {
        size_t len, cap, byte_size;
        void *data;
    } vec_t;

    // Creates a vector with a given length, capacity and size (in bytes)
    vec_t vec_create_impl(size_t len, size_t cap, size_t byte_size);

    // Pushes an element to the end of the vector
    void vec_push_back(vec_t *v, const void *elem);

    // Pops out the last element from the vector
    void *vec_pop_back(vec_t *v);

    // Sets the element at the specified index in the vector
    void vec_set_at(vec_t *v, size_t p, const void *elem);

    // Retrives the element at the specified index in the vector
    void *vec_at(const vec_t *v, size_t p);

    // Retrives the last element in the vector
    void *vec_top(const vec_t *v);

    // Clears out the vector by setting its length to zero
    void vec_clear(vec_t *v);

    // Destroys a given vector
    void vec_destroy(vec_t *v);

#ifdef __cplusplus
}
#endif

#endif /* VEC_H */

#ifndef NUMS_H
#define NUMS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>

    // Structure that holds arrays of numbers
    typedef struct nums
    {
        size_t len, cap;
        double *data;
    } nums_t;

    // Creates a numbers list with a given length and capacity
    nums_t nums_create(size_t len, size_t cap);

    // Creates a copy from another numbers list object (safe to destroy)
    nums_t nums_copy(const nums_t *ns);

    // Pushes a number to the end of the numbers list
    void nums_push_back(nums_t *ns, double x);

    // Pops out the last number from the numbers list
    double nums_pop_back(nums_t *ns);

    // Sets the number at the specified index in the numbers list
    void nums_set_at(nums_t *ns, size_t p, double x);

    // Retrieves the element at the specified index in the numbers list
    double nums_at(const nums_t *ns, size_t p);

    // Retrieves the last number in the numbers list
    double nums_top(const nums_t *ns);

    // Clears out the numbers list by setting its length to zero
    void nums_clear(nums_t *ns);

    // Destroys a given numbers list
    void nums_destroy(nums_t *ns);

#ifdef __cplusplus
}
#endif

#endif /* NUMS_H */

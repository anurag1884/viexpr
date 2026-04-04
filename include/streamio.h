#ifndef STREAMIO_H
#define STREAMIO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "str.h"
#include "vec.h"

#include <stdbool.h>
#include <stdio.h>

    // Reads CSV tokens line-by-line into the tokens array: handling quotes,
    // commas, and whitespace and returns error logs (if any)
    str_t read_csv_tokens(FILE *stream, vec_t *tokens, bool *is_eof,
                          size_t line_no, size_t *skip_lines);

    // Writes an array of tokens as a CSV row
    void write_csv_tokens(FILE *stream, const vec_t *tokens);

#ifdef __cplusplus
}
#endif

#endif /* STREAMIO_H */

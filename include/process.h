#ifndef PROCESS_H
#define PROCESS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "str.h"
#include "vec.h"

#include <stdio.h>

    // Processes a given CSV file, with CSV entries and log entry messages for
    // invalid ones Returns error messages associated to unable to find the
    // necessary headers (ExprID, Expression or Type), which stops parsing
    // unlike other log messages
    str_t process_csv_file(vec_t *csv_messages, FILE *file, vec_t *entries);

#ifdef __cplusplus
}
#endif

#endif /* PROCESS_H */

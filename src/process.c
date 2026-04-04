#include "process.h"
#include "streamio.h"
#include "utils.h"

#include <stdlib.h>

str_t process_csv_file(vec_t *csv_messages, FILE *file, vec_t *entries)
{
    bool is_eof;

    int ch;

    expr_type_t type;

    size_t min_col, id_col, expr_col, type_col, query_id, line_no, skip_lines,
        header_size;

    str_t l1 = str_create(0, 0), l2 = str_create(0, 0), l3 = str_create(0, 0),
          *id_col_it, *type_col_it, message = str_create(0, 0);

    entry_t ent;

    vec_t tokens = vec_create(0, INITIAL_CAP, str_t);

    // Read CSV header
    line_no = 1;
    message = read_csv_tokens(file, &tokens, &is_eof, line_no, &skip_lines);
    if (message.cap == 0)
    {
        find_cols(&tokens, &id_col, &expr_col, &type_col);
        if (id_col == SIZE_MAX)
            l1 = str_format(
                "CSV header does not contain the column 'ExprID' (case-sensitive). Please recheck the column names in the CSV file.\n");
        if (expr_col == SIZE_MAX)
            l2 = str_format(
                "CSV header does not contain the column 'Expression' (case-sensitive). Please recheck the column names in the CSV file.\n");
        if (type_col == SIZE_MAX)
            l3 = str_format(
                "CSV header does not contain the column 'Type' (case-sensitive). Please recheck the column names in the CSV file.\n");
        if (id_col == SIZE_MAX || expr_col == SIZE_MAX || type_col == SIZE_MAX)
        {
            message = str_format("%s%s%s", l1.data, l2.data, l3.data);
            (void)str_pop_back(&message);

            str_destroy(&l1);
            str_destroy(&l2);
            str_destroy(&l3);

            while (tokens.len--)
                str_destroy((str_t *)vec_at(&tokens, tokens.len));
            vec_destroy(&tokens);

            return message;
        }
        min_col = id_col < expr_col ? id_col : expr_col;
        min_col = type_col < min_col ? type_col : min_col;
        header_size = tokens.len;
    }
    else
    {
        while (tokens.len--)
            str_destroy((str_t *)vec_at(&tokens, tokens.len));
        vec_destroy(&tokens);

        return message;
    }

    str_destroy(&message);

    // Read tokens line-by-line in the CSV file and extract the entries
    is_eof = 0;
    skip_lines = 0;
    while (!is_eof)
    {
        line_no++;
        line_no += skip_lines;

        message = read_csv_tokens(file, &tokens, &is_eof, line_no, &skip_lines);
        if (message.cap == 0)
        {
            if (tokens.len == 0)
                continue;
            if (tokens.len >= min_col)
            {
                if (tokens.len == 1 &&
                    is_all_spaces((str_t *)vec_at(&tokens, min_col)))
                    continue;

                id_col_it = (str_t *)vec_at(&tokens, id_col);
                type_col_it = (str_t *)vec_at(&tokens, type_col);
                if (header_size == tokens.len)
                {
                    if (is_valid_id(id_col_it))
                    {
                        query_id = (size_t)strtoul(id_col_it->data, NULL, 10);
                        if (query_id != 0)
                        {
                            type = get_expression_type(type_col_it);
                            if (type == type_unknown)
                            {
                                message = str_format(
                                    "Unknown expression type '%s' at line %zu.",
                                    type_col_it->data, line_no);
                                vec_push_back(csv_messages, &message);
                            }
                            else if (find_entry_by_id(entries, query_id) !=
                                     NULL)
                            {
                                message = str_format(
                                    "Duplicate expression ID '%zu' found at line %zu. Only the first instance will be considered.",
                                    query_id, line_no);
                                vec_push_back(csv_messages, &message);
                            }
                            else
                            {
                                ent = entry_create(
                                    query_id, type,
                                    (str_t *)vec_at(&tokens, expr_col));
                                vec_push_back(entries, &ent);
                                continue;
                            }
                        }
                        else
                        {
                            message =
                                str_format("Invalid ID '%zu' at line %zu.",
                                           query_id, line_no);
                            vec_push_back(csv_messages, &message);
                        }
                    }
                    else
                    {
                        message = str_format(
                            "Invalid non-numeric ID '%s' at line %zu.",
                            id_col_it->data, line_no);
                        vec_push_back(csv_messages, &message);
                    }
                }
                else
                {
                    message = str_format(
                        "Row size does not match header size at line %zu. Row size is %zu while header size is %zu.",
                        line_no, tokens.len, header_size);
                    vec_push_back(csv_messages, &message);
                }
            }
            else
            {
                message = str_format(
                    "Row data does not consist one of: ExprID, Expression and Type at line %zu.",
                    line_no);
                vec_push_back(csv_messages, &message);
            }
        }
        else
        {
            while ((ch = fgetc(file)) != '\r' && ch != '\n' && ch != EOF)
                ;
            vec_push_back(csv_messages, &message);
        }
    }

    while (tokens.len--)
        str_destroy((str_t *)vec_at(&tokens, tokens.len));
    vec_destroy(&tokens);

    message = str_create(0, 0);

    return message;
}

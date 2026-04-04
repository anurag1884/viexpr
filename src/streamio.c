#include "streamio.h"
#include "utils.h"

#include <ctype.h>

str_t read_csv_tokens(FILE *stream, vec_t *tokens, bool *is_eof, size_t line_no,
                      size_t *skip_lines)
{
    bool quote;

    int ch = fgetc(stream), next;

    str_t token, trail_chars = str_create(0, INITIAL_CAP),
                 message = str_create(0, 0);

    *skip_lines = 0;

    while (tokens->len--)
        str_destroy((str_t *)vec_at(tokens, tokens->len));
    vec_clear(tokens);

    // Parse until newline or EOF
    while (ch != '\r' && ch != '\n' && ch != EOF)
    {
        token = str_create(0, INITIAL_CAP);

        // Check for quoted field
        while (ch != EOF && isspace(ch))
        {
            str_push_back(&token, ch);
            ch = fgetc(stream);
        }

        if (ch == '"')
        {
            ch = fgetc(stream);
            str_clear(&token);
            quote = true;
        }
        else
            quote = false;

        // Read token until comma/newline (or closing quote if quoted)
        while ((quote || (!quote && ch != ',' && ch != '\r' && ch != '\n')) &&
               ch != EOF)
        {
            // Handle double quotes inside quoted field
            if (ch == '"')
            {
                if (quote)
                {
                    next = fgetc(stream);
                    if (next == '"')
                        // Escaped quote
                        ch = next;
                    else
                    {
                        // End of quoted field
                        ungetc(next, stream);
                        break;
                    }
                }
                else
                {
                    // Unexpected quote in unquoted field -> invalid CSV
                    while (ch != '\r' && ch != '\n' && ch != EOF && ch != ',')
                    {
                        str_push_back(&token, ch);
                        ch = fgetc(stream);
                    }
                    if (ch == '\r' || ch == '\n')
                        ungetc(ch, stream);
                    str_terminate(&token);

                    message = str_format(
                        "Unexpected quotation encountered in CSV token '%s' at line %zu.",
                        token.data, line_no + *skip_lines);

                    str_destroy(&trail_chars);
                    str_destroy(&token);

                    while (tokens->len--)
                        str_destroy((str_t *)vec_at(tokens, tokens->len));
                    vec_clear(tokens);

                    return message;
                }
            }

            if (ch == '\r' || ch == '\n')
                (*skip_lines)++;

            str_push_back(&token, ch);

            if (ch == '\r')
            {
                ch = fgetc(stream);
                if (ch != '\n')
                    ungetc(ch, stream);
                else
                    str_push_back(&token, ch);
            }

            ch = fgetc(stream);
        }

        // Validate closing quote if field was quoted
        if (quote)
        {
            if (ch == '"')
                // Consume closing quote
                ch = fgetc(stream);
            else
            {
                // Unterminated quote -> error
                str_terminate(&token);

                message = str_format(
                    "Unterminated quotation in CSV token '\"%s' at line %zu.",
                    token.data, line_no);

                str_destroy(&trail_chars);
                str_destroy(&token);

                while (tokens->len--)
                    str_destroy((str_t *)vec_at(tokens, tokens->len));
                vec_clear(tokens);

                return message;
            }
        }

        // If token ends with comma, prepare for next token
        if (ch == ',')
        {
            ch = fgetc(stream);
            // Handle empty token at end of line
            if (ch == '\r' || ch == '\n' || ch == EOF)
            {
                str_terminate(&token);
                vec_push_back(tokens, &token);
                str_clear(&token);
            }
        }
        // Handle invalid trailing characters after quotation
        else if (ch != '\r' && ch != '\n' && ch != EOF)
        {
            str_push_back(&trail_chars, '"');
            while (ch != '\r' && ch != '\n' && ch != EOF && isspace(ch))
            {
                str_push_back(&trail_chars, ch);
                ch = fgetc(stream);
            }
            if (ch == ',')
                ch = fgetc(stream);
            else if (ch != '\r' && ch != '\n' && ch != EOF)
            {
                while (ch != '\r' && ch != '\n' && ch != EOF && ch != ',')
                {
                    str_push_back(&trail_chars, ch);
                    ch = fgetc(stream);
                }
                if (ch == '\r' || ch == '\n')
                    ungetc(ch, stream);
                str_terminate(&token);
                str_terminate(&trail_chars);

                message = str_format(
                    "Invalid trailing characters encountered after quotation in CSV token '\"%s%s' at line %zu.",
                    token.data, trail_chars.data, line_no + *skip_lines);

                str_destroy(&trail_chars);
                str_destroy(&token);

                while (tokens->len--)
                    str_destroy((str_t *)vec_at(tokens, tokens->len));
                vec_clear(tokens);

                return message;
            }
        }

        str_terminate(&token);
        vec_push_back(tokens, &token);
    }

    if (is_eof != NULL)
        *is_eof = (ch == EOF);

    str_destroy(&trail_chars);

    return message;
}

// Writes one CSV token, escaping quotes and adding quotes if needed
void write_csv_token(FILE *stream, const str_t *token)
{
    bool needs_quotes = false;

    char ch;

    size_t len = 0, i;

    // Check if token needs quoting (i.e., if it contains either newline
    // characters, quotes or commas)
    while (len < token->len)
    {
        ch = str_at(token, len);

        if (!needs_quotes &&
            (ch == '\r' || ch == '\n' || ch == '"' || ch == ','))
            needs_quotes = true;
        len++;
    }

    if (needs_quotes)
    {
        fputc('"', stream);
        for (i = 0; i < len; i++)
        {
            ch = str_at(token, i);

            if (ch == '"')
                // Escape quotes
                fprintf(stream, "%s", "\"\"");
            else
                fputc(ch, stream);
        }
        fputc('"', stream);
    }
    else
        for (i = 0; i < len; i++)
            fputc(str_at(token, i), stream);
}

void write_csv_tokens(FILE *stream, const vec_t *tokens)
{
    size_t i;

    for (i = 0; i < tokens->len; i++)
    {
        write_csv_token(stream, (str_t *)vec_at(tokens, i));
        if (i < tokens->len - 1)
            fprintf(stream, ",");
    }
    fprintf(stream, "\n");
}

#include "app.hpp"
#include "eval.h"
#include "nums.h"
#include "process.h"
#include "str.h"
#include "streamio.h"
#include "token.h"
#include "utils.h"

#include <algorithm>
#include <math.h>
#include <parson.h>
#include <tinyfiledialogs.h>

viexpr::app::app(const char *title, int w, int h)
{
    view = webview_create(false, nullptr);

    webview_set_title(view, title);
    webview_set_size(view, w, h, WEBVIEW_HINT_NONE);
}

viexpr::app::~app()
{
    webview_destroy(view);
}

void open_csv_file(const char *id, const char *, void *arg)
{
    webview_t view = (webview_t)arg;

    const char *filterPatterns[] = {"*.csv"};
    const char *path = tinyfd_openFileDialog(
        "Open...",                                      // Title
        "",                                             // Default path
        sizeof filterPatterns / sizeof *filterPatterns, // Number of filters
        filterPatterns,                                 // Filter array
        "CSV files (*.csv)",                            // Filter description
        false // Allow multiple selects
    );

    JSON_Value *path_str_value =
        json_value_init_string(path == nullptr ? "" : path);

    char *serialized_path_str = json_serialize_to_string(path_str_value);
    webview_return(view, id, 0, serialized_path_str);
    json_free_serialized_string(serialized_path_str);

    json_value_free(path_str_value);
}

void read_csv_file(const char *id, const char *req, void *arg)
{
    webview_t view = (webview_t)arg;

    vec_t entries = vec_create(0, INITIAL_CAP, entry_t),
          csv_messages = vec_create(0, INITIAL_CAP, str_t);

    JSON_Value *req_arr_value = json_parse_string(req);
    JSON_Array *req_arr = json_value_get_array(req_arr_value);

    const char *path = json_array_get_string(req_arr, 0);

    FILE *file = fopen(path, "r");

    JSON_Value *res_obj_value = json_value_init_object();
    JSON_Object *res_obj = json_value_get_object(res_obj_value);

    int status = 0;

    if (file != nullptr)
    {
        str_t header_message = process_csv_file(&csv_messages, file, &entries);

        if (header_message.cap == 0)
        {
            JSON_Value *csv_messages_arr_value = json_value_init_array();
            JSON_Array *csv_messages_arr =
                json_value_get_array(csv_messages_arr_value);
            for (size_t i = 0; i < csv_messages.len; i++)
            {
                str_t *message = (str_t *)vec_at(&csv_messages, i);

                json_array_append_string(csv_messages_arr, message->data);

                str_destroy(message);
            }
            vec_destroy(&csv_messages);

            fclose(file);

            std::sort(
                (entry_t *)entries.data, (entry_t *)entries.data + entries.len,
                [](const entry_t &a, const entry_t &b) { return a.id < b.id; });

            JSON_Value *entries_arr_value = json_value_init_array();
            JSON_Array *entries_arr = json_value_get_array(entries_arr_value);
            for (size_t i = 0; i < entries.len; i++)
            {
                entry_t *ent = (entry_t *)vec_at(&entries, i);

                JSON_Value *entry_obj_value = json_value_init_object();
                JSON_Object *entry_obj = json_value_get_object(entry_obj_value);

                json_object_set_number(entry_obj, "id", (double)ent->id);
                json_object_set_number(entry_obj, "type", (double)ent->type);
                json_object_set_string(entry_obj, "expr", ent->expr.data);

                json_array_append_value(entries_arr, entry_obj_value);

                entry_destroy(ent);
            }
            vec_destroy(&entries);

            json_object_set_value(res_obj, "entries", entries_arr_value);
            json_object_set_value(res_obj, "csv_messages",
                                  csv_messages_arr_value);
        }
        else
        {
            status = 1;
            str_t message = str_format(
                "Failed to read header in the provided in CSV file at path \"%s\" due to reason: %s.",
                path, header_message.data);
            tinyfd_messageBox(
                "CSV Header Read Failure", // aTitle: The title of the window
                message.data,              // aMessage: The body text
                "ok",    // aDialogType: "ok", "okcancel", "yesno", or
                         // "yesnocancel"
                "error", // aIconType: "info", "warning", "error", or "question"
                1        // aDefaultButton: 0 for cancel/no, 1 for ok/yes
            );
            str_destroy(&message);
        }
    }
    else
    {
        status = 1;
        str_t message = str_format(
            "Failed to open the provided CSV file at path \"%s\" for reading.\n\nEither the file no longer exists or this program does not have sufficient permissions for reading it.",
            path);
        tinyfd_messageBox(
            "Read Access Failure", // aTitle: The title of the window
            message.data,          // aMessage: The body text
            "ok",    // aDialogType: "ok", "okcancel", "yesno", or "yesnocancel"
            "error", // aIconType: "info", "warning", "error", or "question"
            1        // aDefaultButton: 0 for cancel/no, 1 for ok/yes
        );
        str_destroy(&message);
    }

    json_value_free(req_arr_value);

    char *serialized_res_obj = json_serialize_to_string(res_obj_value);
    webview_return(view, id, status, serialized_res_obj);
    json_free_serialized_string(serialized_res_obj);

    json_value_free(res_obj_value);
}

void save_csv_file(const char *id, const char *, void *arg)
{
    webview_t view = (webview_t)arg;

    const char *filterPatterns[] = {"*.csv"};
    const char *path = tinyfd_saveFileDialog(
        "Save As...",                                   // Title
        "",                                             // Default path
        sizeof filterPatterns / sizeof *filterPatterns, // Number of filters
        filterPatterns,                                 // Filter array
        "CSV files (*.csv)"                             // Filter description
    );

    const char *new_path = nullptr;

    char *extended_path = nullptr;
    if (path != nullptr)
    {
        if (!strrchr(path, '.'))
        {
            extended_path = (char *)malloc(strlen(path) + sizeof ".csv");
            if (extended_path != nullptr)
            {
                strcpy(extended_path, path);
                strcat(extended_path, ".csv");
            }
        }

        new_path = extended_path == nullptr ? path : extended_path;
    }
    JSON_Value *path_str_value =
        json_value_init_string(new_path == nullptr ? "" : new_path);
    free(extended_path);

    char *serialized_path_str = json_serialize_to_string(path_str_value);
    webview_return(view, id, 0, serialized_path_str);
    json_free_serialized_string(serialized_path_str);
    json_value_free(path_str_value);
}

void write_csv_file(const char *id, const char *req, void *arg)
{
    webview_t view = (webview_t)arg;

    JSON_Value *req_arr_value = json_parse_string(req);
    JSON_Array *req_arr = json_value_get_array(req_arr_value);

    JSON_Object *params_obj = json_array_get_object(req_arr, 0);

    const char *path = json_object_get_string(params_obj, "path");

    int status = 0;

    FILE *file = fopen(path, "w");

    if (file != nullptr)
    {
        JSON_Array *entries_arr = json_object_get_array(params_obj, "entries");

        str_t token;

        vec_t tokens = vec_create(0, INITIAL_CAP, str_t);

        token = str_ccopy("ExprID");
        vec_push_back(&tokens, &token);

        token = str_ccopy("Expression");
        vec_push_back(&tokens, &token);

        token = str_ccopy("Type");
        vec_push_back(&tokens, &token);

        write_csv_tokens(file, &tokens);

        while (tokens.len--)
            str_destroy((str_t *)vec_at(&tokens, tokens.len));

        for (size_t i = 0; i < json_array_get_count(entries_arr); i++)
        {
            vec_clear(&tokens);

            JSON_Object *entry_obj = json_array_get_object(entries_arr, i);

            token = str_format("%zu",
                               (size_t)json_object_get_number(entry_obj, "id"));
            vec_push_back(&tokens, &token);

            token = str_format("%s", json_object_get_string(entry_obj, "expr"));
            vec_push_back(&tokens, &token);

            token = str_format("%s", json_object_get_string(entry_obj, "type"));
            vec_push_back(&tokens, &token);

            write_csv_tokens(file, &tokens);

            while (tokens.len--)
                str_destroy((str_t *)vec_at(&tokens, tokens.len));
        }
        fclose(file);

        vec_destroy(&tokens);
    }
    else
    {
        status = 1;
        str_t message = str_format(
            "Failed to open the provided CSV file at path \"%s\" for writing.\n\nEither the file is being accessed by another program or this program does not have sufficient permissions for writing to it.",
            path);
        tinyfd_messageBox(
            "Write Access Failure", // aTitle: The title of the window
            message.data,           // aMessage: The body text
            "ok",    // aDialogType: "ok", "okcancel", "yesno", or
                     // "yesnocancel"
            "error", // aIconType: "info", "warning", "error", or "question"
            1        // aDefaultButton: 0 for cancel/no, 1 for ok/yes
        );
        str_destroy(&message);
    }

    json_value_free(req_arr_value);
    webview_return(view, id, status, "");
}

void validate_expression(const char *id, const char *req, void *arg)
{
    webview_t view = (webview_t)arg;

    JSON_Value *req_arr_value = json_parse_string(req);
    JSON_Array *req_arr = json_value_get_array(req_arr_value);

    JSON_Object *params_obj = json_array_get_object(req_arr, 0);

    expr_type_t type = (expr_type_t)json_object_get_number(params_obj, "type");

    JSON_Array *tokens_arr = json_object_get_array(params_obj, "expr");
    size_t tokens_arr_count = json_array_get_count(tokens_arr);

    vec_t expr_tokens = vec_create(0, tokens_arr_count, token_t);
    for (size_t i = 0; i < tokens_arr_count; i++)
    {
        JSON_Object *token_obj = json_array_get_object(tokens_arr, i);

        token_type_t t_type =
            (token_type_t)json_object_get_number(token_obj, "type");

        const char *t_token = json_object_get_string(token_obj, "token");

        str_t t_token_s = str_ccopy(t_token);

        token_t t = {t_type, t_token_s};

        vec_push_back(&expr_tokens, &t);
    }

    webview_return(view, id, 0,
                   (type == type_infix ? is_valid_infix(&expr_tokens)
                                       : is_valid_postfix(&expr_tokens))
                       ? "true"
                       : "false");

    while (expr_tokens.len--)
        token_destroy((token_t *)vec_at(&expr_tokens, expr_tokens.len));
    vec_destroy(&expr_tokens);

    json_value_free(req_arr_value);
}

void tokenize_expression(const char *id, const char *req, void *arg)
{
    webview_t view = (webview_t)arg;

    JSON_Value *req_arr_value = json_parse_string(req);
    JSON_Array *req_arr = json_value_get_array(req_arr_value);

    const char *expr = json_array_get_string(req_arr, 0);

    str_t expr_s = str_ccopy(expr);
    vec_t expr_tokens = expr_to_tokens(&expr_s);
    str_destroy(&expr_s);

    JSON_Value *tokens_arr_value = json_value_init_array();
    JSON_Array *tokens_arr = json_value_get_array(tokens_arr_value);
    for (size_t i = 0; i < expr_tokens.len; i++)
    {
        token_t *token = (token_t *)vec_at(&expr_tokens, i);

        JSON_Value *token_obj_value = json_value_init_object();
        JSON_Object *token_obj = json_value_get_object(token_obj_value);
        json_object_set_number(token_obj, "type", (double)token->type);
        json_object_set_string(token_obj, "token", token->token.data);
        json_array_append_value(tokens_arr, token_obj_value);

        token_destroy(token);
    }
    vec_destroy(&expr_tokens);

    char *serialized_tokens_arr = json_serialize_to_string(tokens_arr_value);
    webview_return(view, id, 0, serialized_tokens_arr);
    json_free_serialized_string(serialized_tokens_arr);

    json_value_free(req_arr_value);
    json_value_free(tokens_arr_value);
}

const char *fp_err_to_str(double x)
{
    if (isnan(x))
        return "nan";
    if (isinf(x))
        return signbit(x) ? "-inf" : "+inf";
    return "";
}

void evaluate_expression(const char *id, const char *req, void *arg)
{
    webview_t view = (webview_t)arg;

    JSON_Value *req_arr_value = json_parse_string(req);
    JSON_Array *req_arr = json_value_get_array(req_arr_value);

    JSON_Object *params_obj = json_array_get_object(req_arr, 0);

    JSON_Array *tokens_arr = json_object_get_array(params_obj, "expr");
    size_t tokens_arr_count = json_array_get_count(tokens_arr);

    vec_t expr_tokens = vec_create(0, tokens_arr_count, token_t);
    for (size_t i = 0; i < tokens_arr_count; i++)
    {
        JSON_Object *token_obj = json_array_get_object(tokens_arr, i);

        token_type_t t_type =
            (token_type_t)json_object_get_number(token_obj, "type");

        const char *t_token = json_object_get_string(token_obj, "token");

        str_t t_token_s = str_ccopy(t_token);

        token_t t = {t_type, t_token_s};

        vec_push_back(&expr_tokens, &t);
    }

    expr_type_t type = (expr_type_t)json_object_get_number(params_obj, "type");

    JSON_Value *res_obj_value = json_value_init_object();
    JSON_Object *res_obj = json_value_get_object(res_obj_value);

    vec_t postfix = vec_create(0, 0, token_t);
    vec_t out_postfix = vec_create(0, INITIAL_CAP, token_t);

    str_t postfix_op_hist = str_create(0, INITIAL_CAP);
    vec_t number_stack_hist = vec_create(0, INITIAL_CAP, nums_t);

    vec_t infix_op_stack_hist = vec_create(0, INITIAL_CAP, str_t);
    vec_t postfix_hist = vec_create(0, INITIAL_CAP, vec_t);

    str_t message = str_create(0, 0);

    size_t max_stack_size;

    double res;

    if (type == type_postfix)
        // Postfix expressions are forwarded for evaluation as-is
        postfix = expr_tokens;
    else if (type == type_infix)
    {
        // Infix expressions are first converted to postfix and then
        // forwarded for evaluation
        message = infix_to_postfix(&expr_tokens, &out_postfix,
                                   &infix_op_stack_hist, &postfix_hist);

        if (message.cap == 0)
        {
            postfix = out_postfix;

            {
                JSON_Value *postfix_arr_value = json_value_init_array();
                JSON_Array *postfix_arr =
                    json_value_get_array(postfix_arr_value);
                for (size_t i = 0; i < postfix.len; i++)
                {
                    token_t *t = (token_t *)vec_at(&postfix, i);

                    JSON_Value *token_obj_value = json_value_init_object();
                    JSON_Object *token_obj =
                        json_value_get_object(token_obj_value);

                    json_object_set_number(token_obj, "type", (double)t->type);
                    json_object_set_string(token_obj, "token", t->token.data);

                    json_array_append_value(postfix_arr, token_obj_value);
                }
                json_object_set_value(res_obj, "postfix", postfix_arr_value);
            }

            {
                JSON_Value *infix_op_stack_hist_arr_value =
                    json_value_init_array();
                JSON_Array *infix_op_stack_hist_arr =
                    json_value_get_array(infix_op_stack_hist_arr_value);
                max_stack_size = 0;
                for (size_t i = 0; i < infix_op_stack_hist.len; i++)
                {
                    str_t *ops = (str_t *)vec_at(&infix_op_stack_hist, i);

                    json_array_append_string(infix_op_stack_hist_arr,
                                             ops->data);
                    max_stack_size =
                        ops->len > max_stack_size ? ops->len : max_stack_size;

                    str_destroy(ops);
                }
                json_object_set_number(res_obj, "max_infix_stack_size",
                                       (double)max_stack_size);
                json_object_set_value(res_obj, "infix_op_stack_hist",
                                      infix_op_stack_hist_arr_value);
                vec_destroy(&infix_op_stack_hist);
            }

            {
                JSON_Value *postfix_hist_arr_value = json_value_init_array();
                JSON_Array *postfix_hist_arr =
                    json_value_get_array(postfix_hist_arr_value);
                for (size_t i = 0; i < postfix_hist.len; i++)
                {
                    vec_t *hist = (vec_t *)vec_at(&postfix_hist, i);

                    JSON_Value *postfix_arr_value = json_value_init_array();
                    JSON_Array *postfix_arr =
                        json_value_get_array(postfix_arr_value);
                    for (size_t j = 0; j < hist->len; j++)
                    {
                        token_t *t = (token_t *)vec_at(hist, j);

                        JSON_Value *token_obj_value = json_value_init_object();
                        JSON_Object *token_obj =
                            json_value_get_object(token_obj_value);

                        json_object_set_number(token_obj, "type",
                                               (double)t->type);
                        json_object_set_string(token_obj, "token",
                                               t->token.data);

                        json_array_append_value(postfix_arr, token_obj_value);
                    }
                    json_array_append_value(postfix_hist_arr,
                                            postfix_arr_value);

                    while (hist->len--)
                        token_destroy((token_t *)vec_at(hist, hist->len));
                    vec_destroy(hist);
                }
                json_object_set_value(res_obj, "postfix_hist",
                                      postfix_hist_arr_value);
                vec_destroy(&postfix_hist);
            }
        }
        else
            goto failure;
    }
    json_value_free(req_arr_value);

    // Evaluate the postfix expression
    message =
        postfix_evaluate(&postfix, &res, &postfix_op_hist, &number_stack_hist);

    while (expr_tokens.len--)
        token_destroy((token_t *)vec_at(&expr_tokens, expr_tokens.len));
    vec_destroy(&expr_tokens);

    while (out_postfix.len--)
        token_destroy((token_t *)vec_at(&out_postfix, out_postfix.len));
    vec_destroy(&out_postfix);

    if (message.cap == 0)
    {
        if (isnan(res) || isinf(res))
            json_object_set_string(res_obj, "res", fp_err_to_str(res));
        else
            json_object_set_number(res_obj, "res", res);

        json_object_set_string(res_obj, "postfix_op_hist",
                               postfix_op_hist.data);
        str_destroy(&postfix_op_hist);

        JSON_Value *number_stack_hist_arr_value = json_value_init_array();
        JSON_Array *number_stack_hist_arr =
            json_value_get_array(number_stack_hist_arr_value);
        max_stack_size = 0;
        for (size_t i = 0; i < number_stack_hist.len; i++)
        {
            nums_t *numbers = (nums_t *)vec_at(&number_stack_hist, i);

            JSON_Value *numbers_list_value = json_value_init_array();
            JSON_Array *numbers_list = json_value_get_array(numbers_list_value);
            for (size_t j = 0; j < numbers->len; j++)
            {
                double num = nums_at(numbers, j);
                if (isnan(num) || isinf(num))
                    json_array_append_string(numbers_list, fp_err_to_str(num));
                else
                    json_array_append_number(numbers_list, num);
            }
            max_stack_size =
                numbers->len > max_stack_size ? numbers->len : max_stack_size;
            json_array_append_value(number_stack_hist_arr, numbers_list_value);

            nums_destroy(numbers);
        }
        json_object_set_number(res_obj, "max_postfix_stack_size",
                               max_stack_size);
        json_object_set_value(res_obj, "number_stack_hist",
                              number_stack_hist_arr_value);
        vec_destroy(&number_stack_hist);
    }

failure:
    json_object_set_string(res_obj, "message",
                           message.data == nullptr ? "" : message.data);
    str_destroy(&message);

    char *serialized_res_obj = json_serialize_to_string(res_obj_value);
    webview_return(view, id, 0, serialized_res_obj);
    json_free_serialized_string(serialized_res_obj);

    json_value_free(res_obj_value);
}

void viexpr::app::setup()
{
    // Load the webpage onto the webview instance
    webview_set_html(view,
#include "templates/index.txt"
    );

    webview_bind(view, "open_csv_file", open_csv_file, view);
    webview_bind(view, "read_csv_file", read_csv_file, view);
    webview_bind(view, "save_csv_file", save_csv_file, view);
    webview_bind(view, "write_csv_file", write_csv_file, view);

    webview_bind(view, "validate_expression", validate_expression, view);
    webview_bind(view, "tokenize_expression", tokenize_expression, view);
    webview_bind(view, "evaluate_expression", evaluate_expression, view);
}

void viexpr::app::run()
{
    webview_run(view);
}

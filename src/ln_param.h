/*
 * Copyright (c) 2018-2020 Zhixu Zhao
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _LN_PARAM_H_
#define _LN_PARAM_H_

#include <stdarg.h>
#include "ln_list.h"

/* clang-format off */
enum ln_param_type {
    /* NULL should always be the first type */
    LN_PARAM_INVALID = -1,
    LN_PARAM_NULL = 0,
    LN_PARAM_STRING,
    LN_PARAM_NUMBER,
    LN_PARAM_BOOL,
    LN_PARAM_ARRAY_STRING,
    LN_PARAM_ARRAY_NUMBER,
    LN_PARAM_ARRAY_BOOL,
    LN_PARAM_TYPE_SIZE
    /* LN_PARAM_TYPE_SIZE should always be the last type */
};
typedef enum ln_param_type ln_param_type;

struct ln_param_entry {
    char          *arg_name;
    ln_param_type  type;
    int            array_len;
    double         value_double;
    float          value_float;
    int            value_int;
    ln_bool        value_bool;
    char          *value_string;
    char         **value_array_string;
    double        *value_array_double;
    float         *value_array_float;
    int           *value_array_int;
    ln_bool       *value_array_bool;
};
typedef struct ln_param_entry ln_param_entry;
/* clang-format on */

#ifdef __cplusplus
LN_CPPSTART
#endif

ln_param_entry *ln_param_entry_create(const char *arg_name, ln_param_type type);
void ln_param_entry_free(ln_param_entry *entry);
const char *ln_param_type_name(ln_param_type type);
ln_param_entry *ln_param_entry_copy(const ln_param_entry *pe);

void ln_param_vset(ln_param_entry *entry, va_list ap);
void ln_param_set(ln_param_entry *entry, ...);
void ln_param_set_null(ln_param_entry *entry);
void ln_param_set_bool(ln_param_entry *entry, ln_bool bool_value);
void ln_param_set_string(ln_param_entry *entry, const char *string);
void ln_param_set_satu_number(ln_param_entry *entry, double number);
void ln_param_set_satu_array_number(ln_param_entry *entry, int array_len,
                                    const double *array_number);
void ln_param_set_satu_array_double(ln_param_entry *entry, int array_len,
                                    const double *array_number);
void ln_param_set_satu_array_float(ln_param_entry *entry, int array_len, const float *array_number);
void ln_param_set_satu_array_int(ln_param_entry *entry, int array_len, const int *array_number);
void ln_param_set_array_string(ln_param_entry *entry, int array_len, const char **array_string);
void ln_param_set_array_bool(ln_param_entry *entry, int array_len, const ln_bool *array_bool);
/* TODO: add prepend version */
ln_list *ln_param_list_append_empty(ln_list *list, const char *arg_name, ln_param_type ptype);
ln_list *ln_param_list_append_string(ln_list *list, const char *arg_name, const char *string);
ln_list *ln_param_list_append_number(ln_list *list, const char *arg_name, double number);
ln_list *ln_param_list_append_double(ln_list *list, const char *arg_name, double number);
ln_list *ln_param_list_append_float(ln_list *list, const char *arg_name, float number);
ln_list *ln_param_list_append_int(ln_list *list, const char *arg_name, int number);
ln_list *ln_param_list_append_bool(ln_list *list, const char *arg_name, ln_bool bool_value);
ln_list *ln_param_list_append_null(ln_list *list, const char *arg_name);
ln_list *ln_param_list_append_array_string(ln_list *list, const char *arg_name, int array_len,
                                           const char **array_string);
ln_list *ln_param_list_append_array_number(ln_list *list, const char *arg_name, int array_len,
                                           const double *array_number);
ln_list *ln_param_list_append_array_double(ln_list *list, const char *arg_name, int array_len,
                                           const double *array_number);
ln_list *ln_param_list_append_array_float(ln_list *list, const char *arg_name, int array_len,
                                          const float *array_number);
ln_list *ln_param_list_append_array_int(ln_list *list, const char *arg_name, int array_len,
                                        const int *array_int);
ln_list *ln_param_list_append_array_bool(ln_list *list, const char *arg_name, int array_len,
                                         const ln_bool *array_bool);
void ln_param_list_free(ln_list *list);
ln_list *ln_param_list_copy(ln_list *list);
ln_param_entry *ln_param_list_find(ln_list *list, const char *arg_name);
ln_param_entry *ln_param_list_find2(ln_list *list, const char *arg_name1, const char *arg_name2);
int ln_param_list_length(ln_list *list);
int ln_param_list_unique_arg_name(ln_list *list, char *buf, const char *prefix);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif /* _LN_PARAM_H_ */

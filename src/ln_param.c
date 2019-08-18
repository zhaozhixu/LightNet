/*
 * Copyright (c) 2018-2019 Zhao Zhixu
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

#include <string.h>
#include <assert.h>
#include <limits.h>
#include <float.h>
#include <stdarg.h>

#include "ln_param.h"
#include "ln_util.h"
#include "ln_msg.h"

ln_param_entry *ln_param_entry_create(const char *arg_name, ln_param_type type)
{
    ln_param_entry *entry;

    assert(type >= LN_PARAM_NULL && type <= LN_PARAM_INVALID);
    entry = ln_alloc(sizeof(ln_param_entry));
    entry->arg_name = ln_strdup(arg_name);
    entry->type = type;
    entry->array_len = 0;
    entry->value_string = NULL;
    entry->value_double = 0;
    entry->value_float = 0;
    entry->value_int = 0;
    entry->value_bool = LN_FALSE;
    entry->value_array_string = NULL;
    entry->value_array_double = NULL;
    entry->value_array_float = NULL;
    entry->value_array_int = NULL;
    entry->value_array_bool = NULL;

    return entry;
}

void ln_param_entry_free(ln_param_entry *entry)
{
    ln_free(entry->arg_name);
    ln_free(entry->value_string);
    if (entry->type == LN_PARAM_ARRAY_STRING) {
        int i;
        for (i = 0; i < entry->array_len; i++)
            ln_free(entry->value_array_string[i]);
    }
    ln_free(entry->value_array_string);
    ln_free(entry->value_array_double);
    ln_free(entry->value_array_float);
    ln_free(entry->value_array_int);
    ln_free(entry->value_array_bool);
    ln_free(entry);
}

void ln_param_set_bool(ln_param_entry *entry, ln_bool bool)
{
    assert(entry);
    entry->value_bool = bool;
}

void ln_param_set_null(ln_param_entry *entry)
{
    assert(entry);
    entry->type = LN_PARAM_NULL;
}

void ln_param_set_satu_number(ln_param_entry *entry, double number)
{
    assert(entry);

    entry->value_double = number;

    /* use saturation in case of overflow */
    if (number >= FLT_MAX)
        entry->value_float = FLT_MAX;
    else if (number <= FLT_MIN)
        entry->value_float = FLT_MIN;
    else
        entry->value_float = (float)number;

    if (number >= INT_MAX)
        entry->value_int = INT_MAX;
    else if (number <= INT_MIN)
        entry->value_int = INT_MIN;
    else
        entry->value_int = (int)number;
}

void ln_param_set_string(ln_param_entry *entry, const char *string)
{
    assert(entry);
    ln_free(entry->value_string);
    entry->value_string = ln_strdup(string);
}

void ln_param_set_satu_array_number(ln_param_entry *entry, int array_len,
                                    const double *array_number)
{
    int i;
    assert(entry);
    assert(array_len > 0);

    ln_free(entry->value_array_double);
    ln_free(entry->value_array_float);
    ln_free(entry->value_array_int);
    entry->array_len = array_len;
    entry->value_array_double = ln_alloc(sizeof(double)*array_len);
    entry->value_array_float = ln_alloc(sizeof(float)*array_len);
    entry->value_array_int = ln_alloc(sizeof(int)*array_len);

    memmove(entry->value_array_double, array_number, sizeof(double)*array_len);

    /* use saturation in case of overflow */
    for (i = 0; i < array_len; i++) {
        if (array_number[i] >= FLT_MAX)
            entry->value_array_float[i] = FLT_MAX;
        else if (array_number[i] <= FLT_MIN)
            entry->value_array_float[i] = FLT_MIN;
        else
            entry->value_array_float[i] = (float)array_number[i];
    }

    for (i = 0; i < array_len; i++) {
        if (array_number[i] >= INT_MAX)
            entry->value_array_int[i] = INT_MAX;
        else if (array_number[i] <= INT_MIN)
            entry->value_array_int[i] = INT_MIN;
        else
            entry->value_array_int[i] = (int)array_number[i];
    }
}

void ln_param_set_satu_array_double(ln_param_entry *entry, int array_len,
                                    const double *array_number)
{
    ln_param_set_satu_array_number(entry, array_len, array_number);
}

void ln_param_set_satu_array_float(ln_param_entry *entry, int array_len,
                                   const float *array_number)
{
    int i;
    assert(entry);
    assert(array_len > 0);

    ln_free(entry->value_array_double);
    ln_free(entry->value_array_float);
    ln_free(entry->value_array_int);
    entry->array_len = array_len;
    entry->value_array_double = ln_alloc(sizeof(double)*array_len);
    entry->value_array_float = ln_alloc(sizeof(float)*array_len);
    entry->value_array_int = ln_alloc(sizeof(int)*array_len);

    memmove(entry->value_array_float, array_number, sizeof(float)*array_len);

    for (i = 0; i < array_len; i++)
        entry->value_array_double[i] = (double)array_number[i];

    /* use saturation in case of overflow */
    for (i = 0; i < array_len; i++) {
        if (array_number[i] >= INT_MAX)
            entry->value_array_int[i] = INT_MAX;
        else if (array_number[i] <= INT_MIN)
            entry->value_array_int[i] = INT_MIN;
        else
            entry->value_array_int[i] = (int)array_number[i];
    }
}

void ln_param_set_satu_array_int(ln_param_entry *entry, int array_len,
                                 const int *array_number)
{
    int i;
    assert(entry);
    assert(array_len > 0);

    ln_free(entry->value_array_double);
    ln_free(entry->value_array_float);
    ln_free(entry->value_array_int);
    entry->array_len = array_len;
    entry->value_array_double = ln_alloc(sizeof(double)*array_len);
    entry->value_array_float = ln_alloc(sizeof(float)*array_len);
    entry->value_array_int = ln_alloc(sizeof(int)*array_len);

    memmove(entry->value_array_int, array_number, sizeof(int)*array_len);

    for (i = 0; i < array_len; i++) {
        entry->value_array_double[i] = (double)array_number[i];
        entry->value_array_float[i] = (float)array_number[i];
    }
}

void ln_param_set_array_string(ln_param_entry *entry, int array_len,
                               const char **array_string)
{
    int i;
    assert(entry);
    assert(array_len > 0);

    for (i = 0; i < entry->array_len; i++)
        ln_free(entry->value_array_string[i]);
    ln_free(entry->value_array_string);
    entry->array_len = array_len;
    entry->value_array_string = ln_alloc(sizeof(char *)*array_len);
    for (i = 0; i < array_len; i++) {
        entry->value_array_string[i] = ln_strdup(array_string[i]);
    }
}

void ln_param_set_array_bool(ln_param_entry *entry, int array_len,
                             const ln_bool *array_bool)
{
    assert(entry);
    assert(array_len > 0);

    ln_free(entry->value_array_bool);
    entry->array_len = array_len;
    entry->value_array_bool = ln_alloc(sizeof(ln_bool)*array_len);
    memmove(entry->value_array_bool, array_bool, sizeof(ln_bool)*array_len);
}

void ln_param_vset(ln_param_entry *entry, va_list ap)
{
    int len;
    ln_bool *array_bool;
    double *array_double;
    char **array_string;
    ln_bool bool_value;
    double number;
    char *string;

    switch (entry->type) {
    case LN_PARAM_ARRAY_BOOL:
        array_bool = va_arg(ap, ln_bool *);
        len = va_arg(ap, int);
        ln_param_set_array_bool(entry, len, array_bool);
        break;
    case LN_PARAM_ARRAY_NUMBER:
        array_double = va_arg(ap, double *);
        len = va_arg(ap, int);
        ln_param_set_satu_array_number(entry, len, array_double);
        break;
    case LN_PARAM_ARRAY_STRING:
        array_string = va_arg(ap, char **);
        len = va_arg(ap, int);
        ln_param_set_array_string(entry, len, (const char **)array_string);
        break;
    case LN_PARAM_BOOL:
        bool_value = va_arg(ap, ln_bool);
        ln_param_set_bool(entry, bool_value);
        break;
    case LN_PARAM_NULL:
        ln_param_set_null(entry);
        break;
    case LN_PARAM_NUMBER:
        number = va_arg(ap, double);
        ln_param_set_satu_number(entry, number);
        break;
    case LN_PARAM_STRING:
        string = va_arg(ap, char *);
        ln_param_set_string(entry, string);
        break;
    case LN_PARAM_INVALID:
    default:
        assert(0 && "unsupported ln_param_type");
        break;
    }
}

void ln_param_set(ln_param_entry *entry, ...)
{
    va_list ap;
    va_start(ap, entry);
    ln_param_vset(entry, ap);
    va_end(ap);
}

ln_list *ln_param_list_append_empty(ln_list *list, const char *arg_name,
                                    ln_param_type ptype)
{
    ln_param_entry *entry;

    entry = ln_param_entry_create(arg_name, ptype);
    list = ln_list_append(list, entry);
    return list;
}

ln_list *ln_param_list_append_string(ln_list *list, const char *arg_name,
                                     const char *string)
{
    ln_param_entry *entry;

    entry = ln_param_entry_create(arg_name, LN_PARAM_STRING);
    entry->value_string = ln_strdup(string);
    list = ln_list_append(list, entry);
    return list;
}

ln_list *ln_param_list_append_number(ln_list *list, const char *arg_name,
                                     double number)
{
    ln_param_entry *entry;

    entry = ln_param_entry_create(arg_name, LN_PARAM_NUMBER);
    entry->value_double = number;

    /* use saturation in case of overflow */
    if (number >= FLT_MAX)
        entry->value_float = FLT_MAX;
    else if (number <= FLT_MIN)
        entry->value_float = FLT_MIN;
    else
        entry->value_float = (float)number;

    if (number >= INT_MAX)
        entry->value_int = INT_MAX;
    else if (number <= INT_MIN)
        entry->value_int = INT_MIN;
    else
        entry->value_int = (int)number;

    list = ln_list_append(list, entry);
    return list;
}

ln_list *ln_param_list_append_double(ln_list *list, const char *arg_name,
                                     double number)
{
    return ln_param_list_append_number(list, arg_name, number);
}

ln_list *ln_param_list_append_float(ln_list *list, const char *arg_name,
                                    float number)
{
    ln_param_entry *entry;

    entry = ln_param_entry_create(arg_name, LN_PARAM_NUMBER);
    entry->value_double = (double)number;
    entry->value_float = number;

    /* use saturation in case of overflow */
    if (number >= INT_MAX)
        entry->value_int = INT_MAX;
    else if (number <= INT_MIN)
        entry->value_int = INT_MIN;
    else
        entry->value_int = (int)number;

    list = ln_list_append(list, entry);
    return list;
}

ln_list *ln_param_list_append_int(ln_list *list, const char *arg_name,
                                  int number)
{
    ln_param_entry *entry;

    entry = ln_param_entry_create(arg_name, LN_PARAM_NUMBER);
    entry->value_double = (double)number;
    entry->value_float = (float)number;
    entry->value_int = number;

    list = ln_list_append(list, entry);
    return list;
}

ln_list *ln_param_list_append_bool(ln_list *list, const char *arg_name,
                                   ln_bool bool_value)
{
    ln_param_entry *entry;

    entry = ln_param_entry_create(arg_name, LN_PARAM_BOOL);
    entry->value_bool = bool_value;
    list = ln_list_append(list, entry);
    return list;
}

ln_list *ln_param_list_append_null(ln_list *list, const char *arg_name)
{
    ln_param_entry *entry;

    entry = ln_param_entry_create(arg_name, LN_PARAM_NULL);
    list = ln_list_append(list, entry);
    return list;
}

ln_list *ln_param_list_append_array_string(ln_list *list, const char *arg_name,
                                           int array_len,
                                           const char **array_string)
{
    ln_param_entry *entry;

    entry = ln_param_entry_create(arg_name, LN_PARAM_ARRAY_STRING);
    ln_param_set_array_string(entry, array_len, array_string);
    list = ln_list_append(list, entry);
    return list;
}

ln_list *ln_param_list_append_array_number(ln_list *list, const char *arg_name,
                                           int array_len,
                                           const double *array_number)
{
    ln_param_entry *entry;

    entry = ln_param_entry_create(arg_name, LN_PARAM_ARRAY_NUMBER);
    ln_param_set_satu_array_number(entry, array_len, array_number);
    list = ln_list_append(list, entry);
    return list;
}

ln_list *ln_param_list_append_array_double(ln_list *list, const char *arg_name,
                                           int array_len,
                                           const double *array_number)
{
    return ln_param_list_append_array_number(list, arg_name, array_len,
                                             array_number);
}

ln_list *ln_param_list_append_array_float(ln_list *list, const char *arg_name,
                                          int array_len,
                                          const float *array_number)
{
    ln_param_entry *entry;

    entry = ln_param_entry_create(arg_name, LN_PARAM_ARRAY_NUMBER);
    ln_param_set_satu_array_float(entry, array_len, array_number);
    list = ln_list_append(list, entry);
    return list;
}

ln_list *ln_param_list_append_array_int(ln_list *list, const char *arg_name,
                                        int array_len, const int *array_number)
{
    ln_param_entry *entry;

    entry = ln_param_entry_create(arg_name, LN_PARAM_ARRAY_NUMBER);
    ln_param_set_satu_array_int(entry, array_len, array_number);
    list = ln_list_append(list, entry);
    return list;
}

ln_list *ln_param_list_append_array_bool(ln_list *list, const char *arg_name,
                                         int array_len,
                                         const ln_bool *array_bool)
{
    ln_param_entry *entry;

    assert(array_len >= 0);
    entry = ln_param_entry_create(arg_name, LN_PARAM_ARRAY_BOOL);
    ln_param_set_array_bool(entry, array_len, array_bool);
    list = ln_list_append(list, entry);
    return list;
}

ln_list *ln_param_list_copy(ln_list *list)
{
    ln_list *new_list = NULL;
    ln_param_entry *entry;
    LN_LIST_FOREACH(entry, list) {
        switch (entry->type) {
        case LN_PARAM_ARRAY_BOOL:
            new_list = ln_param_list_append_array_bool(new_list, entry->arg_name,
                                                       entry->array_len,
                                                       entry->value_array_bool);
            break;
        case LN_PARAM_ARRAY_NUMBER:
            new_list = ln_param_list_append_array_number(new_list,
                                                         entry->arg_name,
                                                         entry->array_len,
                                                         entry->value_array_double);
            break;
        case LN_PARAM_ARRAY_STRING:
            new_list = ln_param_list_append_array_string(new_list,
                                                         entry->arg_name,
                                                         entry->array_len,
                                                         (const char **)entry->value_array_string);
            break;
        case LN_PARAM_BOOL:
            new_list = ln_param_list_append_bool(new_list, entry->arg_name,
                                                 entry->value_bool);
            break;
        case LN_PARAM_NULL:
            new_list = ln_param_list_append_null(new_list, entry->arg_name);
            break;
        case LN_PARAM_NUMBER:
            new_list = ln_param_list_append_number(new_list, entry->arg_name,
                                                   entry->value_double);
            break;
        case LN_PARAM_STRING:
            new_list = ln_param_list_append_string(new_list, entry->arg_name,
                                                   entry->value_string);
            break;
        case LN_PARAM_INVALID:
        default:
            assert(0 && "unsupported ln_param_type");
            break;
        }
    }
    return new_list;
}

static void param_entry_free_wrapper(void *p)
{
    ln_param_entry_free(p);
}

void ln_param_list_free(ln_list *list)
{
    ln_list_free_deep(list, param_entry_free_wrapper);
}

static int find_by_arg_name(const void *data1, const void *data2)
{
    const ln_param_entry *p1 = data1;
    const ln_param_entry *p2 = data2;

    return strcmp(p1->arg_name, p2->arg_name);
}

ln_param_entry *ln_param_list_find(ln_list *list, const char *arg_name)
{
    ln_param_entry cmp_entry;
    ln_param_entry *result_entry;

    cmp_entry.arg_name = (char *)arg_name;
    result_entry = ln_list_find_custom(list, &cmp_entry, find_by_arg_name);

    return result_entry;
}

ln_param_entry *ln_param_list_find2(ln_list *list, const char *arg_name1,
                                    const char *arg_name2)
{
    ln_param_entry cmp_entry;
    ln_param_entry *result_entry;

    cmp_entry.arg_name = ln_strcat_delim_alloc(arg_name1, arg_name2, '_');
    result_entry = ln_list_find_custom(list, &cmp_entry, find_by_arg_name);
    ln_free(cmp_entry.arg_name);

    return result_entry;
}

int ln_param_list_length(ln_list *list)
{
    return ln_list_length(list);
}

int ln_param_list_unique_arg_name(ln_list *list, char *buf, const char *prefix)
{
    ln_param_entry *pe;
    int max_idx = -1;
    int idx;
    size_t prefix_len = strlen(prefix);

    if (prefix_len >= LN_MAX_NAME_LEN)
        ln_msg_inter_error("prefix '%s' length exceeds LN_MAX_NAME_LEN", prefix);
    LN_LIST_FOREACH(pe, list) {
        if (!ln_is_prefix_plus_number(pe->arg_name, prefix))
            continue;
        idx = atoi(&pe->arg_name[prefix_len]);
        max_idx = max_idx < idx ? idx : max_idx;
    }
    max_idx++;
    if (ln_digit_num(max_idx) + prefix_len >= LN_MAX_NAME_LEN)
        ln_msg_inter_error("result '%s%d' length exceeds LN_MAX_NAME_LEN",
                           prefix, max_idx);
    snprintf(buf, LN_MAX_NAME_LEN, "%s%d", prefix, max_idx);

    return max_idx;
}

const char *ln_param_type_name(ln_param_type type)
{
    switch (type) {
    case LN_PARAM_NULL:
        return "LN_PARAM_NULL";
    case LN_PARAM_STRING:
        return "LN_PARAM_STRING";
    case LN_PARAM_NUMBER:
        return "LN_PARAM_NUMBER";
    case LN_PARAM_BOOL:
        return "LN_PARAM_BOOL";
    case LN_PARAM_ARRAY_STRING:
        return "LN_PARAM_ARRAY_STRING";
    case LN_PARAM_ARRAY_NUMBER:
        return "LN_PARAM_ARRAY_NUMBER";
    case LN_PARAM_ARRAY_BOOL:
        return "LN_PARAM_ARRAY_BOOL";
    case LN_PARAM_INVALID:
        return "LN_PARAM_INVALID";
    default:
        assert(0 && "unsupported ln_param_type");
    }
}

ln_param_entry *ln_param_entry_copy(const ln_param_entry *pe)
{
    ln_param_entry *new_pe;

    new_pe = ln_param_entry_create(pe->arg_name, pe->type);
    new_pe->array_len = pe->array_len;
    new_pe->value_string = pe->value_string ?
        ln_strdup(pe->value_string) : NULL;
    new_pe->value_double = pe->value_double;
    new_pe->value_float = pe->value_float;
    new_pe->value_int = pe->value_int;
    new_pe->value_bool = pe->value_bool;
    if (pe->value_array_string) {
        new_pe->value_array_string = ln_alloc(sizeof(char *)*pe->array_len);
        for (int i = 0; i < pe->array_len; i++)
            new_pe->value_array_string[i] = ln_strdup(pe->value_array_string[i]);
    } else {
        new_pe->value_array_string = NULL;
    }
    new_pe->value_array_double = pe->value_array_double ?
        ln_clone(pe->value_array_double, sizeof(double)*pe->array_len) : NULL;
    new_pe->value_array_float = pe->value_array_float ?
        ln_clone(pe->value_array_float, sizeof(float)*pe->array_len) : NULL;
    new_pe->value_array_int = pe->value_array_int ?
        ln_clone(pe->value_array_int, sizeof(int)*pe->array_len) : NULL;
    new_pe->value_array_bool = pe->value_array_bool ?
        ln_clone(pe->value_array_bool, sizeof(ln_bool)*pe->array_len) : NULL;;

    return new_pe;
}

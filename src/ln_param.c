/*
 * Copyright (c) 2018 Zhao Zhixu
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
#include "ln_param.h"
#include "ln_util.h"

ln_param_entry *ln_param_entry_create(const char *arg_name, ln_param_type type)
{
    ln_param_entry *entry;

    assert(type >= LN_PARAM_NULL && type < LN_PARAM_INVALID);
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
                                   ln_bool bool)
{
    ln_param_entry *entry;

    entry = ln_param_entry_create(arg_name, LN_PARAM_BOOL);
    entry->value_bool = bool;
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
                                           int array_len, char **array_string)
{
    ln_param_entry *entry;
    int i;

    assert(array_len >= 0);
    entry = ln_param_entry_create(arg_name, LN_PARAM_ARRAY_STRING);
    entry->array_len = array_len;
    entry->value_array_string = ln_alloc(sizeof(char *)*array_len);
    for (i = 0; i < array_len; i++) {
        entry->value_array_string[i] = ln_strdup(array_string[i]);
    }
    list = ln_list_append(list, entry);
    return list;
}

ln_list *ln_param_list_append_array_number(ln_list *list, const char *arg_name,
                                           int array_len, double *array_number)
{
    ln_param_entry *entry;
    int i;

    assert(array_len >= 0);
    entry = ln_param_entry_create(arg_name, LN_PARAM_ARRAY_NUMBER);
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

    list = ln_list_append(list, entry);
    return list;
}

ln_list *ln_param_list_append_array_double(ln_list *list, const char *arg_name,
                                           int array_len, double *array_number)
{
    return ln_param_list_append_array_number(list, arg_name, array_len, array_number);
}

ln_list *ln_param_list_append_array_float(ln_list *list, const char *arg_name,
                                          int array_len, float *array_number)
{
    ln_param_entry *entry;
    int i;

    assert(array_len >= 0);
    entry = ln_param_entry_create(arg_name, LN_PARAM_ARRAY_NUMBER);
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

    list = ln_list_append(list, entry);
    return list;
}

ln_list *ln_param_list_append_array_int(ln_list *list, const char *arg_name,
                                        int array_len, int *array_int)
{
    ln_param_entry *entry;
    int i;

    assert(array_len >= 0);
    entry = ln_param_entry_create(arg_name, LN_PARAM_ARRAY_NUMBER);
    entry->array_len = array_len;
    entry->value_array_double = ln_alloc(sizeof(double)*array_len);
    entry->value_array_float = ln_alloc(sizeof(float)*array_len);
    entry->value_array_int = ln_alloc(sizeof(int)*array_len);

    memmove(entry->value_array_int, array_int, sizeof(int)*array_len);

    for (i = 0; i < array_len; i++) {
        entry->value_array_double[i] = (double)array_int[i];
        entry->value_array_float[i] = (float)array_int[i];
    }

    list = ln_list_append(list, entry);
    return list;
}

ln_list *ln_param_list_append_array_bool(ln_list *list, const char *arg_name,
                                         int array_len, ln_bool *array_bool)
{
    ln_param_entry *entry;

    assert(array_len >= 0);
    entry = ln_param_entry_create(arg_name, LN_PARAM_ARRAY_BOOL);
    entry->array_len = array_len;
    entry->value_array_bool = ln_alloc(sizeof(ln_bool)*array_len);
    memmove(entry->value_array_bool, array_bool, sizeof(ln_bool)*array_len);
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
                                                         entry->value_array_string);
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

    cmp_entry.arg_name = ln_strdup(arg_name);
    result_entry = ln_list_find_custom(list, &cmp_entry, find_by_arg_name);
    ln_free(cmp_entry.arg_name);

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

char *ln_param_list_create_arg_name(ln_list *list, const char *prefix)
{
    ln_param_entry *pe;
    int max_idx = -1;
    int idx;
    char *buf;
    size_t prefix_len = strlen(prefix);
    size_t buf_len = prefix_len + LN_MAX_NAME_SUBFIX;

    buf = ln_alloc(sizeof(char)*buf_len);
    LN_LIST_FOREACH(pe, list) {
        if (!ln_is_prefix_plus_number(pe->arg_name, prefix))
            continue;
        idx = atoi(&pe->arg_name[prefix_len]);
        max_idx = max_idx < idx ? idx : max_idx;
    }
    snprintf(buf, buf_len, "%s%d", prefix, max_idx+1);
    return buf;
}

const char *ln_param_type_name(ln_param_type type)
{
    switch (type) {
    case LN_PARAM_NULL:
        return "null";
    case LN_PARAM_STRING:
        return "String";
    case LN_PARAM_NUMBER:
        return "Number";
    case LN_PARAM_BOOL:
        return "Boolean";
    case LN_PARAM_ARRAY_STRING:
        return "String Array";
    case LN_PARAM_ARRAY_NUMBER:
        return "Number Array";
    case LN_PARAM_ARRAY_BOOL:
        return "Boolean Array";
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

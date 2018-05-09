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
#include "ln_param.h"
#include "ln_util.h"

static ln_param_entry *ln_param_entry_create(const char *arg_name,
                                             ln_param_type type)
{
     ln_param_entry *entry;

     assert(type >= LN_PARAM_NULL && type < LN_PARAM_INVALID);
     entry = ln_alloc(sizeof(ln_param_entry));
     entry->arg_name = ln_alloc(sizeof(char)*(strlen(arg_name)+1));
     strcpy(entry->arg_name, arg_name);
     entry->type = type;
     entry->array_len = 0;
     entry->value_string = NULL;
     entry->value_double = 0;
     entry->value_int = 0;
     entry->value_bool = LN_FALSE;
     entry->value_array_string = NULL;
     entry->value_array_double = NULL;
     entry->value_array_int = NULL;
     entry->value_array_bool = NULL;

     return entry;
}

static void ln_param_entry_free(ln_param_entry *entry)
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
     ln_free(entry->value_array_int);
     ln_free(entry->value_array_bool);
     ln_free(entry);
}

ln_param_table *ln_param_table_append_string(ln_param_table *table,
					     const char *arg_name,
					     const char *string)
{
     ln_param_entry *entry;

     entry = ln_param_entry_create(arg_name, LN_PARAM_STRING);
     entry->value_string = ln_alloc(sizeof(char)*(strlen(string)+1));
     strcpy(entry->value_string, string);
     table = ln_list_append(table, entry);
     return table;
}

ln_param_table *ln_param_table_append_number(ln_param_table *table,
					     const char *arg_name,
					     double number)
{
     ln_param_entry *entry;

     entry = ln_param_entry_create(arg_name, LN_PARAM_NUMBER);
     entry->value_double = number;
     /* use saturation in case of overflow */
     if (number >= INT_MAX)
          entry->value_int = INT_MAX;
     else if (number <= INT_MIN)
          entry->value_int = INT_MIN;
     else
          entry->value_int = (int)number;

     table = ln_list_append(table, entry);
     return table;
}

ln_param_table *ln_param_table_append_bool(ln_param_table *table,
					   const char *arg_name,
					   ln_bool bool)
{
     ln_param_entry *entry;

     entry = ln_param_entry_create(arg_name, LN_PARAM_BOOL);
     entry->value_bool = bool;
     table = ln_list_append(table, entry);
     return table;
}

ln_param_table *ln_param_table_append_null(ln_param_table *table,
					   const char *arg_name)
{
     ln_param_entry *entry;

     entry = ln_param_entry_create(arg_name, LN_PARAM_NULL);
     table = ln_list_append(table, entry);
     return table;
}

ln_param_table *ln_param_table_append_array_string(ln_param_table *table,
                                                  const char *arg_name,
                                                  int array_len,
                                                  char **array_string)
{
     ln_param_entry *entry;
     int i;

     assert(array_len >= 0);
     entry = ln_param_entry_create(arg_name, LN_PARAM_ARRAY_STRING);
     entry->array_len = array_len;
     entry->value_array_string = ln_alloc(sizeof(char *)*array_len);
     for (i = 0; i < array_len; i++) {
          entry->value_array_string[i] =
               ln_alloc(sizeof(char)*(strlen(array_string[i])+1));
          strcpy(entry->value_array_string[i], array_string[i]);
     }
     table = ln_list_append(table, entry);
     return table;
}

ln_param_table *ln_param_table_append_array_number(ln_param_table *table,
                                                  const char *arg_name,
                                                  int array_len,
                                                  double *array_number)
{
     ln_param_entry *entry;
     int i;

     assert(array_len >= 0);
     entry = ln_param_entry_create(arg_name, LN_PARAM_ARRAY_NUMBER);
     entry->array_len = array_len;
     entry->value_array_double = ln_alloc(sizeof(double)*array_len);
     entry->value_array_int = ln_alloc(sizeof(int)*array_len);
     memmove(entry->value_array_double, array_number, sizeof(double)*array_len);
     for (i = 0; i < array_len; i++) {
          /* use saturation in case of overflow */
          if (array_number[i] >= INT_MAX)
               entry->value_array_int[i] = INT_MAX;
          else if (array_number[i] <= INT_MIN)
               entry->value_array_int[i] = INT_MIN;
          else
               entry->value_array_int[i] = (int)array_number[i];
     }

     table = ln_list_append(table, entry);
     return table;
}

ln_param_table *ln_param_table_append_array_bool(ln_param_table *table,
                                                  const char *arg_name,
                                                  int array_len,
                                                  ln_bool *array_bool)
{
     ln_param_entry *entry;

     assert(array_len >= 0);
     entry = ln_param_entry_create(arg_name, LN_PARAM_ARRAY_BOOL);
     entry->array_len = array_len;
     entry->value_array_bool = ln_alloc(sizeof(ln_bool)*array_len);
     memmove(entry->value_array_bool, array_bool, sizeof(ln_bool)*array_len);
     table = ln_list_append(table, entry);
     return table;
}


static void param_entry_free_wrapper(void *p)
{
     ln_param_entry_free(p);
}

void ln_param_table_free(ln_param_table *table)
{
     ln_list_free_deep(table, param_entry_free_wrapper);
}

static int find_by_arg_name(void *data1, void *data2)
{
     ln_param_entry *p1, *p2;

     p1 = (ln_param_entry *)data1;
     p2 = (ln_param_entry *)data2;
     return strcmp(p1->arg_name, p2->arg_name);
}

ln_param_entry *ln_param_table_find_by_arg_name(ln_param_table *table,
						char *arg_name)
{
     ln_param_entry cmp_entry;
     ln_param_entry *result_entry;

     cmp_entry.arg_name = arg_name;
     result_entry = ln_list_find_custom(table, &cmp_entry, find_by_arg_name);

     return result_entry;
}

int ln_param_table_length(ln_param_table *table)
{
     return ln_list_length(table);
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

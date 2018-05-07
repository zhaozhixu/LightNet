#include <string.h>
#include <assert.h>
#include "ln_param.h"
#include "ln_util.h"

static ln_param_entry *ln_param_entry_create(const char *arg_name,
                                             ln_param_type type)
{
     ln_param_entry *entry;

     entry = ln_alloc(sizeof(ln_param_entry));
     entry->arg_name = ln_alloc(sizeof(char)*(strlen(arg_name)+1));
     strcpy(entry->arg_name, arg_name);
     entry->type = type;
     entry->array_len = 0;
     switch (entry->type) {
     case LN_PARAM_NULL:
          break;
     case LN_PARAM_STRING:
          entry->value_string = NULL;
          break;
     case LN_PARAM_NUMBER:
          entry->value_number = 0;
          break;
     case LN_PARAM_BOOL:
          entry->value_bool = LN_FALSE;
          break;
     case LN_PARAM_ARRAY_STRING:
          entry->value_array_string = NULL;
          break;
     case LN_PARAM_ARRAY_NUMBER:
          entry->value_array_number = NULL;
          break;
     case LN_PARAM_ARRAY_BOOL:
          entry->value_array_bool = NULL;
          break;
     default:
          assert(0 && "unsupported ln_param_type");
     }

     return entry;
}

static void ln_param_entry_free(ln_param_entry *entry)
{
     ln_free(entry->arg_name);
     switch (entry->type) {
     case LN_PARAM_STRING:
          ln_free(entry->value_string);
          break;
     case LN_PARAM_NUMBER:
          break;
     case LN_PARAM_BOOL:
          break;
     case LN_PARAM_ARRAY_STRING:
          int i;
          for (i = 0; i < entry->array_len; i++)
               ln_free(entry->value_array_string[i]);
          ln_free(entry->value_array_string);
          break;
     case LN_PARAM_ARRAY_NUMBER:
          ln_free(entry->value_array_number);
          break;
     case LN_PARAM_ARRAY_BOOL:
          ln_free(entry->value_array_bool);
          break;
     case LN_PARAM_NULL:
          break;
     default:
          assert(0 && "unsupported ln_param_type");
     }
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
     entry->value_number = number;
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
                                                  const char **array_string)
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
     entry->value_array_number = ln_alloc(sizeof(double)*array_len);
     memmove(entry->value_array_number, array_number, sizeof(double)*array_len);
     table = ln_list_append(table, entry);
}

ln_param_table *ln_param_table_append_array_bool(ln_param_table *table,
                                                  const char *arg_name,
                                                  int array_len,
                                                  ln_bool *array_bool)
{
     ln_param_entry *entry;
     int i;

     assert(array_len >= 0);
     entry = ln_param_entry_create(arg_name, LN_PARAM_ARRAY_BOOL);
     entry->array_len = array_len;
     entry->value_array_bool = ln_alloc(sizeof(ln_bool)*array_len);
     memmove(entry->value_array_bool, array_bool, sizeof(ln_bool)*array_len);
     table = ln_list_append(table, entry);
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

ln_param_entry *ln_param_table_find_by_arg_name(const ln_param_table *table,
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
          return "String Array"
     case LN_PARAM_ARRAY_NUMBER:
          return "Number Array";
     case LN_PARAM_ARRAY_BOOL:
          return "Boolean Array";
     default:
          assert(0 && "unsupported ln_param_type");
     }
}

#include <string.h>
#include "ln_param.h"
#include "ln_util.h"

ln_param_entry *ln_param_entry_create(const char *arg_name, ln_param_type type)
{
     ln_param_entry *entry;

     entry = ln_alloc(sizeof(ln_param_entry));
     entry->arg_name = arg_name;
     entry->value_string = NULL;
     entry->value_number = 0;
     entry->value_bool = LN_FALSE;
     entry->type = type;

     return entry;
}

void ln_param_entry_free(ln_param_entry *entry)
{
     ln_free(entry->value_string);
     ln_free(entry);
}

void ln_param_entry_set_string(ln_param_entry *entry, const char *string)
{
     entry->value_string = ln_alloc(sizeof(char)*(strlen(string)+1));
     strcpy(entry->value_string, string);
}

void ln_param_entry_set_number(ln_param_entry *entry, double number)
{
     entry->value_number = number;
}

void ln_param_entry_set_bool(ln_param_entry *entry, ln_bool bool)
{
     entry->value_bool = bool;
}

ln_param_table *ln_param_table_append(ln_param_table *table, ln_param_entry *entry)
{
     table = ln_list_append(table, entry);
     return table;
}

ln_param_table *ln_param_table_append_string(ln_param_table *table,
					     const char *arg_name,
					     const char *string)
{
     ln_param_entry *entry;

     entry = ln_param_entry_create(arg_name, LN_PARAM_STRING);
     ln_param_entry_set_string(entry, string);
     table = ln_list_append(table, entry);
     return table;
}

ln_param_table *ln_param_table_append_number(ln_param_table *table,
					     const char *arg_name,
					     double number)
{
     ln_param_entry *entry;

     entry = ln_param_entry_create(arg_name, LN_PARAM_NUMBER);
     ln_param_entry_set_number(entry, number);
     table = ln_list_append(table, entry);
     return table;
}

ln_param_table *ln_param_table_append_bool(ln_param_table *table,
					   const char *arg_name,
					   ln_bool bool)
{
     ln_param_entry *entry;

     entry = ln_param_entry_create(arg_name, LN_PARAM_BOOL);
     ln_param_entry_set_bool(entry, bool);
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

static void param_entry_free_wrapper(void *p)
{
     ln_param_entry_free(p);
}

void ln_param_table_free(ln_param_table *table)
{
     ln_list_free_deep(table, param_entry_free_wrapper);
}

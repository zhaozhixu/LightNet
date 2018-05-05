#include "ln_tensor.h"
#include "ln_util.h"

ln_tensor_entry *ln_tensor_entry_create(const char *name, const char *arg_name,
					tl_tensor *tensor)
{
     ln_tensor_entry *entry;

     entry = ln_alloc(sizeof(ln_tensor_entry));
     entry->name = ln_alloc(sizeof(char)*(strlen(name)+1));
     strcpy(entry->name, name);
     entry->arg_name = ln_alloc(sizeof(char)*(strlen(arg_name)+1));
     strcpy(entry->arg_name, arg_name);
     entry->tensor = tensor;

     return entry;
}

void ln_tensor_entry_free(ln_tensor_entry *entry)
{
     ln_free(entry->name);
     ln_free(entry->arg_name);
     ln_free(entry);
}

ln_tensor_table *ln_tensor_table_append(ln_tensor_table *table, const char *name,
					const char *arg_name, tl_tensor *tensor)
{
     ln_tensor_entry *entry;

     entry = ln_tensor_entry_create(name, arg_name, tensor);
     table = ln_list_append(table, entry);
     return table;
}

static void tensor_entry_free_wrapper(void *p)
{
     ln_tensor_entry_free(p);
}

void ln_tensor_table_free(ln_tensor_table *table)
{
     ln_list_free_deep(table, tensor_entry_free_wrapper);
}

static int find_by_arg_name(void *data1, void *data2)
{
     ln_tensor_entry *t1, *t2;

     t1 = (ln_tensor_entry *)data1;
     t2 = (ln_tensor_entry *)data2;
     return strcmp(t1->arg_name, t2->arg_name);
}

ln_tensor_entry *ln_tensor_table_find_by_arg_name(ln_tensor_table *table,
						  char *arg_name)
{
     ln_tensor_entry cmp_entry;
     ln_tensor_entry *result_entry;

     cmp_entry.arg_name = arg_name;
     result_entry = ln_list_find_custom(table, &cmp_entry, find_by_arg_name);

     return result_entry;
}

static int find_by_name(void *data1, void *data2)
{
     ln_tensor_entry *t1, *t2;

     t1 = (ln_tensor_entry *)data1;
     t2 = (ln_tensor_entry *)data2;
     return strcmp(t1->name, t2->name);
}

ln_tensor_entry *ln_tensor_table_find_by_name(ln_tensor_table *table,
					      char *name)
{
     ln_tensor_entry cmp_entry;
     ln_tensor_entry *result_entry;

     cmp_entry.name = name;
     result_entry = ln_list_find_custom(table, &cmp_entry, find_by_name);

     return result_entry;
}

int ln_tensor_table_length(ln_tensor_table *table)
{
     return ln_list_length(table);
}

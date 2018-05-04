#include "ln_tensor.h"
#include "ln_util.h"

ln_tensor_entry *ln_tensor_entry_create(const char *name, const char *arg_name,
					tl_tensor *tensor)
{
     ln_tensor_entry *entry;

     entry = ln_alloc(sizeof(ln_tensor_entry));
     entry->name = name;
     entry->arg_name = arg_name;
     entry->tensor = tensor;

     return entry;
}

void ln_tensor_entry_free(ln_tensor_entry *entry)
{
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

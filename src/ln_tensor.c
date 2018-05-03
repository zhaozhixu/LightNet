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

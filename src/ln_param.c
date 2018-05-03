#include "ln_param.h"
#include "ln_util.h"

ln_param_entry *ln_param_entry_create(const char *arg_name, tl_param *param)
{
	ln_param_entry *entry;

	entry = ln_alloc(sizeof(ln_param_entry));
	entry->arg_name = arg_name;
	entry->param = param;

	return entry;
}

void ln_param_entry_free(ln_param_entry *entry)
{
	ln_free(entry);
}

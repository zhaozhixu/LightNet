#ifndef _LN_TENSOR_H_
#define _LN_TENSOR_H_

#include "tl_tensor.h"
#include "ln_list.h"

typedef struct ln_tensor_entry ln_tensor_entry;
struct ln_tensor_entry {
     char       *name;
     char       *arg_name;
     tl_tensor  *tensor;
};

typedef ln_list ln_tensor_table;

#ifdef __cplusplus
extern "C" {
#endif

ln_tensor_entry *ln_tensor_entry_create(const char *name, const char *arg_name,
					tl_tensor *tensor);
void ln_tensor_entry_free(ln_tensor_entry *entry);
ln_tensor_table *ln_tensor_table_append(ln_tensor_table *table, const char *name,
					const char *arg_name, tl_tensor *tensor);
void ln_tensor_table_free(ln_tensor_table *table);
ln_tensor_entry *ln_tensor_table_find_by_arg_name(ln_tensor_table *table,
						  char *arg_name);
ln_tensor_entry *ln_tensor_table_find_by_name(ln_tensor_table *table,
					      char *name);
int ln_tensor_table_length(ln_tensor_table *table);

#ifdef __cplusplus
}
#endif

#endif	/* _LN_TENSOR_H_ */

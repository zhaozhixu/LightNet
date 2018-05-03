#ifndef _LN_TENSOR_H_
#define _LN_TENSOR_H_

#include "tl_tensor.h"
#include "ln_list.h"

typedef struct ln_tensor_entry ln_tensor_entry;
struct ln_tensor_entry {
     const char *name;
     const char *arg_name;
     tl_tensor  *tensor;
};

#ifdef __cplusplus
extern "C" {
#endif

ln_tensor_entry *ln_tensor_entry_create(const char *name, const char *arg_name,
					tl_tensor *tensor);
void ln_tensor_entry_free(ln_tensor_entry *entry);

#ifdef __cplusplus
}
#endif

#endif	/* _LN_TENSOR_H_ */

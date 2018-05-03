#ifndef _LN_PARAM_H_
#define _LN_PARAM_H_

#include "ln_list.h"

typedef struct ln_param_entry ln_param_entry;
struct ln_param_entry {
     const char *arg_name;
     void       *param;
};

#ifdef __cplusplus
extern "C" {
#endif

ln_param_entry *ln_param_entry_create(const char *arg_name, void *param);
void ln_param_entry_free(ln_param_entry *entry);

#ifdef __cplusplus
}
#endif

#endif	/* _LN_PARAM_H_ */

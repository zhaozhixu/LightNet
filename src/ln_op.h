#ifndef _LN_OP_H_
#define _LN_OP_H_

#include "ln_tensor.h"
#include "ln_param.h"
#include "ln_error.h"

typedef struct ln_op_arg ln_op_arg;
struct ln_op_arg {
     char            *name;
     char            *optype;
     ln_tensor_table *tensors;
     ln_param_table  *params;
};

typedef void (*ln_op_func) (ln_op_arg *op_arg, ln_error **error);

typedef struct ln_op ln_op;
struct ln_op {
     ln_op_arg   *op_arg;
     ln_op_func   pre_run;
     ln_op_func   run;
     ln_op_func   post_run;
};


#ifdef __cplusplus
extern "C" {
#endif

ln_op_arg *ln_op_arg_create(const char *name, const char *optype,
			    ln_tensor_table *tensors, ln_param_table *params);
void ln_op_arg_free(ln_op_arg *op_arg);
ln_op *ln_op_create(ln_op_arg *op_arg, ln_op_func pre_run, ln_op_func run,
		    ln_op_func post_run);
void ln_op_free(ln_op *op);
tl_tensor *ln_op_list_find_tensor_by_name(ln_list *ops, char *name);

#ifdef __cplusplus
}
#endif


#endif  /* _LN_OP_H_ */

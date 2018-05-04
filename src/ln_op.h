#ifndef _LN_OP_H_
#define _LN_OP_H_

#include "ln_tensor.h"
#include "ln_param.h"

typedef void (*ln_op_func) (ln_tensor_table *tensors, ln_param_table *params);

typedef struct ln_op ln_op;
struct ln_op {
     const char      *name;
     const char      *type_name;
     ln_tensor_table *tensors;
     ln_param_table  *params;
     ln_op_func       run;
     ln_op_func       pre_run;
     ln_op_func       post_run;
};

#endif  /* _LN_OP_H_ */

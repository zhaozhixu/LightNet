#include "ln_op.h"
#include "ln_list.h"

extern ln_op ln_op_slice;
extern ln_op ln_op_maxreduce;
/* end of declarations */

ln_op *ln_init_ops[] = {
     &ln_op_slice,
     &ln_op_maxreduce,
     &ln_op_maxreduce,
     /* end of init ops */
};

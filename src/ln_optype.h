#ifndef _LN_OPTYPE_H_
#define _LN_OPTYPE_H_

#include "ln_op.h"
#include "ln_list.h"

extern ln_op ln_op_slice;

ln_op *ln_init_ops[] = {
     &ln_op_slice
};

ln_list *ln_registered_ops = NULL;

#endif	/* _LN_OPTYPE_H_ */

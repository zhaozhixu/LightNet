#ifndef _LN_OP_H_
#define _LN_OP_H_

#include "ln_optype.h"

typedef struct ln_op ln_op;
struct ln_op {
     const char      *name;
     ln_optype        optype;
     void            *args;
     ln_op_run_func   run;
     ln_op_pre_func   pre;
     ln_op_post_func  post;
};

#endif  /* _LN_OP_H_ */

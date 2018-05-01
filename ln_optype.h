#ifndef _LN_OPTYPE_H_
#define _LN_OPTYPE_H_

typedef enum ln_optype ln_optype;
enum ln_optype {
     LN_SLICE,
     LN_RESHAPE,
     LN_MAXREDUCE,
     LN_MUL,
     LN_TRANSPOSE,
     LN_CONVOLUTION,
     LN_ACTIVATION,
     LN_POOLING
};

typedef void (*ln_op_run_func) (void *args);
typedef void (*ln_op_pre_func) (void *args);
typedef void (*ln_op_post_func) (void *args);



#endif  /*  _LN_OPTYPE_H_ */

#include <assert.h>
#include "ln_op.h"

/*
 * This function should do the parameter checking and tensor memory allocation.
 */
static void maxreduce_pre_run(ln_op_arg *op_arg, ln_error **error)
{

     /* check tensors and parameters */
     /* ...... */

     /* allocate tensor memory in need */
     /* ...... */
}

/*
 * Normally we should only do the calculations here. Operations with memory
 * and such should go in pre_run().
 */
static void maxreduce_run(ln_op_arg *op_arg, ln_error **error)
{

     /* Get tensors and parameters, which should have been checked in pre_run().
        Further errors should be considered as bugs, so we use asserts to catch
        return value. */
     /* ...... */

     /* do the real work */
     /* ...... */
}

/*
 * This function should free all tensor memory pre_run() and run() allocated.
 */
static void maxreduce_post_run(ln_op_arg *op_arg, ln_error **error)
{

     /* free the tensor memory allocated in pre_run() and run() */
     /* ..... */
}

static ln_op_arg op_arg_maxreduce = {
     .name = NULL,
     .optype = "maxreduce",
     .tensors = NULL,
     .params = NULL,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_op_slice = {
     .op_arg = &op_arg_maxreduce,
     .pre_run = maxreduce_pre_run,
     .run = maxreduce_run,
     .post_run = maxreduce_post_run
};

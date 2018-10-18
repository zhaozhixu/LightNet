/*
 * Copyright (c) 2018 Zhao Zhixu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <assert.h>
#include "ln_op.h"

/*
 * This function should do the parameter checking and memory allocation.
 */
static void elew_cuda_pre_run(ln_op_arg *op_arg, ln_error **error)
{

     /* check tensors and parameters */
     /* ...... */

     /* allocate memory in need */
     /* ...... */
}

/*
 * Normally we should only do the calculations here. Operations with memory
 * and such should go in pre_run().
 */
static void elew_cuda_run(ln_op_arg *op_arg, ln_error **error)
{

     /* do the real work */
     /* ...... */
}

/*
 * This function should free all tensor memory pre_run() allocated.
 */
static void elew_cuda_post_run(ln_op_arg *op_arg, ln_error **error)
{

     /* free memory allocated in pre_run() */
     /* ..... */
}

static ln_op_arg op_arg_elew_cuda = {
     .optype = "elew_cuda",
     .mtype_in = LN_MEM_CUDA,
     .mtype_out = LN_MEM_CUDA,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_elew_cuda = {
     .op_arg = &op_arg_elew_cuda,
     .pre_run = elew_cuda_pre_run,
     .static_run = NULL,
     .run = elew_cuda_run,
     .post_run = elew_cuda_post_run
};

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

#include <math.h>
#include <assert.h>
#include "ln_op.h"

static inline int compute_length(int ndim, const int *dims)
{
     int i, len;

     for (i = 0, len = 1; i < ndim; i++)
          len *= dims[i];
     return len;
}

/*
 * This function should do the parameter checking and memory allocation.
 */
static void reshape_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     char *dst_name, *src_name;
     ln_tensor_entry *dst_entry, *src_entry;
     ln_param_entry *dims_entry;
     int tensors_n, params_n;
     int *dims, ndim, i;

     /* check tensors and parameters */
     tensors_n = ln_tensor_list_length(op_arg->tensors_in);
     ln_op_check_tensor_in_len_eq(LN_ERROR, tensors_n, 1);

     tensors_n = ln_tensor_list_length(op_arg->tensors_out);
     ln_op_check_tensor_out_len_eq(LN_ERROR, tensors_n, 1);

     src_name = ln_tensor_list_find_name(op_arg->tensors_in, "src");
     ln_op_check_tensor_in_exist(LN_ERROR, src_name, "src");
     src_entry = ln_tensor_table_find(op_arg->tensor_table, src_name);
     ln_op_check_tensor_defined(LN_ERROR, src_entry);

     dst_name = ln_tensor_list_find_name(op_arg->tensors_out, "dst");
     ln_op_check_tensor_out_exist(LN_ERROR, dst_name, "dst");
     dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
     ln_op_check_tensor_not_defined(LN_ERROR, dst_entry);

     params_n = ln_param_list_length(op_arg->params);
     ln_op_check_param_len_eq(LN_ERROR, params_n, 1);

     dims_entry = ln_param_list_find(op_arg->params, "dims");
     ln_op_check_param_exist(LN_ERROR, dims_entry, "dims");
     ln_op_check_param_type(LN_ERROR, dims_entry, LN_PARAM_ARRAY_NUMBER);

     dims = dims_entry->value_array_int;
     ndim = dims_entry->array_len;
     ln_op_check_param_satisfy_msg(LN_ERROR, ndim > 0,
                                   "\"dims\" array shouldn't be empty");
     for (i = 0; i < ndim; i++)
          ln_op_check_param_satisfy_msg(LN_ERROR, dims[i] > 0,
                                        "\"dims\" array elements should be positive");
     ln_op_check_param_satisfy_msg(LN_ERROR,
                                   src_entry->tensor->len == compute_length(ndim, dims),
                                   "\"src\" tensor length is not equal with requested length");

     /* Allocate memory in need. */
     dst_entry->tensor = tl_tensor_reshape(src_entry->tensor, ndim, dims);

     op_arg->priv = dst_entry->tensor;
}

/*
 * Normally we should only do the calculations here. Operations with memory
 * and such should go in pre_run(). But since this is an "in-place" reshape,
 * operation don't need to allocate data memory for "dst" tensor.
 */
static void reshape_run(ln_op_arg *op_arg, ln_error **error)
{

     /* do the real work */
}

/*
 * This function should free all memory that pre_run() allocated.
 */
static void reshape_post_run(ln_op_arg *op_arg, ln_error **error)
{
     /*
      * Only free the tensor struct, not its data shared with src.
      */
     tl_tensor_free(op_arg->priv);
}

static ln_op_arg op_arg_reshape = {
     .optype = "reshape",
     .mtype_in = LN_MEM_CPU,
     .mtype_out = LN_MEM_CPU,
};

ln_op ln_opimpl_reshape = {
     .op_arg = &op_arg_reshape,
     .pre_run = reshape_pre_run,
     .static_run = NULL,
     .run = reshape_run,
     .post_run = reshape_post_run
};

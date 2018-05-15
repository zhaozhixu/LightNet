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
 * This function should do the parameter checking and tensor memory allocation.
 */
static void transpose_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *src_entry, *dst_entry;
     ln_param_entry *axes_entry;
     int tensors_n, params_n;
     int *axes;

     /* check tensors and parameters */
     tensors_n = ln_tensor_table_length(op_arg->tensors);
     ln_op_check_tensor_len_eq(LN_ERROR, tensors_n, 2);

     src_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "src");
     ln_op_check_tensor_exist(LN_ERROR, src_entry, "src");
     ln_op_check_tensor_defined(LN_ERROR, src_entry);

     dst_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "dst");
     ln_op_check_tensor_exist(LN_ERROR, dst_entry, "dst");
     ln_op_check_tensor_not_defined(LN_ERROR, dst_entry);

     params_n = ln_param_table_length(op_arg->params);
     ln_op_check_param_len_eq(LN_ERROR, params_n, 1);

     axes_entry = ln_param_table_find_by_arg_name(op_arg->params, "axes");
     ln_op_check_param_type(LN_ERROR, axes_entry, LN_PARAM_ARRAY_NUMBER);

     axes = axes_entry->value_array_int;
     int *tmp = ln_alloc(src_entry->tensor->ndim * sizeof(int));
     memset(tmp, 0, src_entry->tensor->ndim * sizeof(int));
     int i;
     for (i = 0; i < src_entry->tensor->ndim; i++)
          tmp[axes[i]] = 1;
     for (i = 0; i < src_entry->tensor->ndim; i++)
          ln_op_check_param_satisfy_msg(LN_ERROR, tmp[i],
                                        "\"axes\" should match \"src\" tensor's shape");
     ln_free(tmp);

     /* allocate tensor memory in need */
     int *d_dims = ln_alloc(src_entry->tensor->ndim * sizeof(int));
     for (i = 0; i < src_entry->tensor->ndim; i++)
          d_dims[i] = src_entry->tensor->dims[axes[i]];
     dst_entry->tensor = tl_tensor_create(NULL, src_entry->tensor->ndim, d_dims,
                                          src_entry->tensor->dtype);
     ln_free(d_dims);

     /* allocate workspace */
     int **workspace = ln_alloc(sizeof(int *) * 2);
     workspace[0] = ln_alloc(sizeof(int) * dst_entry->tensor->ndim *
                             dst_entry->tensor->len);
     workspace[1] = ln_alloc(sizeof(int) * dst_entry->tensor->ndim *
                             dst_entry->tensor->len);
     op_arg->priv = workspace;
}

/*
 * Normally we should only do the calculations here. Operations with memory
 * and such should go in pre_run().
 */
static void transpose_run(ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *src_entry, *dst_entry;
     ln_param_entry *axes_entry;

     /* Get tensors and parameters, which should have been checked in pre_run().
        Further errors should be considered as bugs, so we use asserts to catch
        return value. */
     src_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "src");
     assert(src_entry);
     dst_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "dst");
     assert(dst_entry);
     axes_entry = ln_param_table_find_by_arg_name(op_arg->params, "axes");
     assert(axes_entry);

     /* do the real work */
     tl_tensor_transpose(src_entry->tensor, dst_entry->tensor,
                         axes_entry->value_array_int, op_arg->priv);
}

/*
 * This function should free all tensor memory pre_run() and run() allocated.
 */
static void transpose_post_run(ln_op_arg *op_arg, ln_error **error)
{

     ln_tensor_entry *dst_entry;

     dst_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "dst");
     assert(dst_entry);

     /* free the tensor memory allocated in pre_run() and run() */
     tl_tensor_free_data_too(dst_entry->tensor);

     ln_free(((int **)op_arg->priv)[0]);
     ln_free(((int **)op_arg->priv)[1]);
     ln_free(op_arg->priv);
}

static ln_op_arg op_arg_transpose = {
     .name = NULL,
     .optype = "transpose",
     .tensors = NULL,
     .params = NULL,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_op_transpose = {
     .op_arg = &op_arg_transpose,
     .pre_run = transpose_pre_run,
     .run = transpose_run,
     .post_run = transpose_post_run
};

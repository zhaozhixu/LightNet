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

struct priv_s {
     char      *dst_name;
     tl_tensor *dst_tensor;
     tl_tensor *src_tensor;
};

/*
 * This function should do the parameter checking and memory allocation.
 */
static void reshape_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     char *dst_name, *src_name;
     ln_tensor_entry *dst_entry, *src_entry;
     tl_tensor *dst_tensor;
     ln_param_entry *dims_entry;
     int tensors_n, params_n;
     int *dims, ndim, i;
     struct priv_s *priv;

     /* check tensors and parameters */
     tensors_n = ln_tensor_list_length(op_arg->tensors_in);
     ln_op_check_tensor_in_len_eq(tensors_n, 1);

     tensors_n = ln_tensor_list_length(op_arg->tensors_out);
     ln_op_check_tensor_out_len_eq(tensors_n, 1);

     src_name = ln_tensor_list_find_name(op_arg->tensors_in, "src");
     ln_op_check_tensor_in_exist(src_name, "src");
     src_entry = ln_tensor_table_find(op_arg->tensor_table, src_name);
     ln_op_check_tensor_defined(src_entry, src_name);

     dst_name = ln_tensor_list_find_name(op_arg->tensors_out, "dst");
     ln_op_check_tensor_out_exist(dst_name, "dst");
     dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
     ln_op_check_tensor_not_defined(dst_entry, dst_name);

     params_n = ln_param_list_length(op_arg->params);
     ln_op_check_param_len_eq(params_n, 1);

     dims_entry = ln_param_list_find(op_arg->params, "dims");
     ln_op_check_param_exist(dims_entry, "dims");
     ln_op_check_param_type(dims_entry, LN_PARAM_ARRAY_NUMBER);

     dims = dims_entry->value_array_int;
     ndim = dims_entry->array_len;
     ln_op_check_param_satisfy_msg(ndim > 0,
                                   "\"dims\" array shouldn't be empty");
     for (i = 0; i < ndim; i++)
          ln_op_check_param_satisfy_msg(dims[i] > 0,
                                        "\"dims\" array elements should be positive");
     ln_op_check_param_satisfy_msg(src_entry->tensor->len == compute_length(ndim, dims),
                                   "\"src\" tensor length is not equal with requested length");

     /* define output tensor shape, tensor data should be NULL */
     dst_tensor = tl_tensor_reshape(src_entry->tensor, ndim, dims);
     dst_entry = ln_tensor_entry_create(dst_name, dst_tensor);
     ln_tensor_entry_set_owner(dst_entry, op_arg->tensor_table, src_name);
     ln_tensor_table_insert(op_arg->tensor_table, dst_name, dst_entry);

     /* use op_arg->priv to store private data to be used in other functions */
     priv = ln_alloc(sizeof(struct priv_s));
     priv->dst_name = dst_name;
     priv->dst_tensor = dst_tensor;
     priv->src_tensor = src_entry->tensor;
     op_arg->priv = priv;
}

/* This function runs only once per instance right after memory allocation. */
static void reshape_static_run(ln_op_arg *op_arg, ln_error **error)
{
     struct priv_s *priv;

     priv = op_arg->priv;
     priv->dst_tensor->data = priv->src_tensor->data;
}

/*
 * This function should only do the calculations.
 */
static void reshape_run(ln_op_arg *op_arg, ln_error **error)
{

}

/*
 * This function should undo everything done by pre_run().
 */
static void reshape_post_run(ln_op_arg *op_arg, ln_error **error)
{
     struct priv_s *priv;

     priv = op_arg->priv;
     ln_tensor_table_remove(op_arg->tensor_table, priv->dst_name);
     ln_free(op_arg->priv);
}

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_reshape = {
     .optype = "reshape",
     .mtype_in = LN_MEM_CPU,
     .mtype_out = LN_MEM_CPU,
};

ln_op ln_opimpl_reshape = {
     .op_arg = &op_arg_reshape,
     .pre_run = reshape_pre_run,
     .static_run = reshape_static_run,
     .run = reshape_run,
     .post_run = reshape_post_run
};

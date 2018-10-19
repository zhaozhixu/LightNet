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

struct priv_s {
     tl_tensor *src;
     tl_tensor *dst;
     char      *dst_name;
     int       *axes;
     tl_tensor *workspace;
};

/*
 * This function should do the parameter checking and tensor memory allocation.
 */
static void transpose_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     char *src_name, *dst_name;
     ln_tensor_entry *src_entry, *dst_entry;
     tl_tensor *dst_tensor;
     ln_param_entry *axes_entry;
     int tensors_n, params_n;
     int *axes;
     struct priv_s *priv;

     /* check tensors and parameters */
     tensors_n = ln_tensor_list_length(op_arg->tensors_in);
     ln_op_check_tensor_in_len_eq(LN_ERROR, tensors_n, 1);

     tensors_n = ln_tensor_list_length(op_arg->tensors_out);
     ln_op_check_tensor_out_len_eq(LN_ERROR, tensors_n, 1);

     src_name = ln_tensor_list_find_name(op_arg->tensors_in, "src");
     ln_op_check_tensor_in_exist(LN_ERROR, src_name, "src");
     src_entry = ln_tensor_table_find(op_arg->tensor_table, src_name);
     ln_op_check_tensor_defined(LN_ERROR, src_entry, src_name);

     dst_name = ln_tensor_list_find_name(op_arg->tensors_out, "dst");
     ln_op_check_tensor_out_exist(LN_ERROR, dst_name, "dst");
     dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
     ln_op_check_tensor_not_defined(LN_ERROR, dst_entry, dst_name);

     params_n = ln_param_list_length(op_arg->params);
     ln_op_check_param_len_eq(LN_ERROR, params_n, 1);

     axes_entry = ln_param_list_find(op_arg->params, "axes");
     ln_op_check_param_exist(LN_ERROR, axes_entry, "axes");
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

     int *d_dims = ln_alloc(src_entry->tensor->ndim * sizeof(int));
     for (i = 0; i < src_entry->tensor->ndim; i++)
          d_dims[i] = src_entry->tensor->dims[axes[i]];
     dst_tensor = tl_tensor_create(NULL, src_entry->tensor->ndim, d_dims,
                                   src_entry->tensor->dtype);
     dst_entry = ln_tensor_entry_create(dst_name, dst_tensor);
     ln_tensor_table_insert(op_arg->tensor_table, dst_name, dst_entry);
     ln_free(d_dims);

     priv = ln_alloc(sizeof(struct priv_s));
     priv->src = src_entry->tensor;
     priv->dst = dst_entry->tensor;
     priv->dst_name = dst_name;
     priv->axes = axes;
     priv->workspace = NULL;
     op_arg->priv = priv;
}

/* This function runs only once per instance right after memory allocation. */
static void transpose_static_run(ln_op_arg *op_arg, ln_error **error)
{
     struct priv_s *priv;

     priv = op_arg->priv;
     priv->workspace = tl_tensor_zeros(1, (int[]){priv->dst->ndim*priv->dst->len*2}, TL_INT32);
}

/*
 * This function should only do the calculations.
 */
static void transpose_run(ln_op_arg *op_arg, ln_error **error)
{
     struct priv_s *priv;

     priv = op_arg->priv;
     tl_tensor_transpose(priv->src, priv->dst, priv->axes, priv->workspace);
}

/*
 * This function should free all tensor memory pre_run() allocated.
 */
static void transpose_post_run(ln_op_arg *op_arg, ln_error **error)
{
     struct priv_s *priv;

     priv = op_arg->priv;
     tl_tensor_free_data_too(priv->workspace);
     ln_tensor_table_remove(op_arg->tensor_table, priv->dst_name);
     ln_free(op_arg->priv);
}

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_transpose = {
     .optype = "transpose",
     .mtype_in = LN_MEM_CPU,
     .mtype_out = LN_MEM_CPU,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_transpose = {
     .op_arg = &op_arg_transpose,
     .pre_run = transpose_pre_run,
     .static_run = transpose_static_run,
     .run = transpose_run,
     .post_run = transpose_post_run
};

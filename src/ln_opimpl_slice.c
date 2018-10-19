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
     int        axis;
     int        start;
     int        len;
};

/*
 * This function should do the parameter checking and memory allocation.
 */
static void slice_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     char *dst_name, *src_name;
     ln_tensor_entry *dst_entry, *src_entry;
     tl_tensor *dst_tensor;
     ln_param_entry *axis_entry, *start_entry, *len_entry;
     int tensors_n, params_n;
     int axis, start, len;
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
     ln_op_check_tensor_not_defined(LN_WARNING, dst_entry, dst_name);

     params_n = ln_param_list_length(op_arg->params);
     ln_op_check_param_len_eq(LN_ERROR, params_n, 3);

     axis_entry = ln_param_list_find(op_arg->params, "axis");
     ln_op_check_param_exist(LN_ERROR, axis_entry, "axis");
     ln_op_check_param_type(LN_ERROR, axis_entry, LN_PARAM_NUMBER);

     start_entry = ln_param_list_find(op_arg->params, "start");
     ln_op_check_param_exist(LN_ERROR, start_entry, "start");
     ln_op_check_param_type(LN_ERROR, start_entry, LN_PARAM_NUMBER);

     len_entry = ln_param_list_find(op_arg->params, "len");
     ln_op_check_param_exist(LN_ERROR, len_entry, "len");
     ln_op_check_param_type(LN_ERROR, len_entry, LN_PARAM_NUMBER);

     axis = axis_entry->value_int;
     start = start_entry->value_int;
     len = len_entry->value_int;
     ln_op_check_param_satisfy(LN_ERROR,
			      axis >= 0 && axis < src_entry->tensor->ndim);
     ln_op_check_param_satisfy(LN_ERROR,
			      start >= 0 && start < src_entry->tensor->dims[axis]);
     ln_op_check_param_satisfy(LN_ERROR,
			      len > 0 && len <= src_entry->tensor->dims[axis]);
     ln_op_check_param_satisfy(LN_ERROR,
			      len + start <= src_entry->tensor->dims[axis]);

     /* define output tensor shape, tensor data should be NULL */
     dst_tensor = tl_tensor_create_slice(NULL, src_entry->tensor, axis, len,
                                         src_entry->tensor->dtype);
     dst_entry = ln_tensor_entry_create(dst_name, dst_tensor);
     ln_tensor_table_insert(op_arg->tensor_table, dst_name, dst_entry);

     priv = ln_alloc(sizeof(struct priv_s));
     priv->src = src_entry->tensor;
     priv->dst = dst_entry->tensor;
     priv->dst_name = dst_name;
     priv->axis = axis;
     priv->start = start;
     priv->len = len;
     op_arg->priv = priv;
}

/*
 * This function should only do the calculations.
 */
static void slice_run(ln_op_arg *op_arg, ln_error **error)
{
     struct priv_s *priv;

     priv = op_arg->priv;
     tl_tensor_slice(priv->src, priv->dst, priv->axis, priv->start, priv->len);
}

/*
 * This function should free all memory that pre_run() allocated.
 */
static void slice_post_run(ln_op_arg *op_arg, ln_error **error)
{
     struct priv_s *priv;

     priv = op_arg->priv;
     ln_tensor_table_remove(op_arg->tensor_table, priv->dst_name);
     ln_free(op_arg->priv);
}

static ln_op_arg op_arg_slice = {
     .optype = "slice",
     .mtype_in = LN_MEM_CPU,
     .mtype_out = LN_MEM_CPU,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_slice = {
     .op_arg = &op_arg_slice,
     .pre_run = slice_pre_run,
     .static_run = NULL,
     .run = slice_run,
     .post_run = slice_post_run
};

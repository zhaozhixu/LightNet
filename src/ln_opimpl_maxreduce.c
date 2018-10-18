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
     tl_tensor *arg;
     char      *dst_name;
     char      *arg_name;
     int        axis;
};

/*
 * This function should do the parameter checking and tensor memory allocation.
 */
static void maxreduce_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     char *src_name, *dst_name, *arg_name;
     ln_tensor_entry *src_entry, *dst_entry, *arg_entry;
     tl_tensor *dst_tensor, *arg_tensor;
     ln_param_entry *axis_entry;
     int tensors_n, params_n;
     int axis;
     struct priv_s *priv;

     /* check tensors and parameters */
     tensors_n = ln_tensor_list_length(op_arg->tensors_in);
     ln_op_check_tensor_in_len_eq(LN_ERROR, tensors_n, 1);

     tensors_n = ln_tensor_list_length(op_arg->tensors_out);
     ln_op_check_tensor_out_len_ge(LN_ERROR, tensors_n, 1);
     ln_op_check_tensor_out_len_le(LN_ERROR, tensors_n, 2);

     src_name = ln_tensor_list_find_name(op_arg->tensors_in, "src");
     ln_op_check_tensor_in_exist(LN_ERROR, src_name, "src");
     src_entry = ln_tensor_table_find(op_arg->tensor_table, src_name);
     ln_op_check_tensor_defined(LN_ERROR, src_entry);

     dst_name = ln_tensor_list_find_name(op_arg->tensors_out, "dst");
     ln_op_check_tensor_out_exist(LN_ERROR, dst_name, "dst");
     dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
     ln_op_check_tensor_not_defined(LN_ERROR, dst_entry);

     /* "arg" is an optional parameter */
     arg_name = ln_tensor_list_find_name(op_arg->tensors_out, "arg");
     if (arg_name) {
          arg_entry = ln_tensor_table_find(op_arg->tensor_table, arg_name);
          ln_op_check_tensor_not_defined(LN_ERROR, arg_entry);
     }

     params_n = ln_param_list_length(op_arg->params);
     ln_op_check_param_len_eq(LN_ERROR, params_n, 1);

     axis_entry = ln_param_list_find(op_arg->params, "axis");
     ln_op_check_param_exist(LN_ERROR, axis_entry, "axis");
     ln_op_check_param_type(LN_ERROR, axis_entry, LN_PARAM_NUMBER);

     axis = axis_entry->value_int;
     ln_op_check_param_satisfy(LN_ERROR,
                               axis >= 0 && axis < src_entry->tensor->ndim);

     /* define output tensor shape, tensor data should be NULL */
     dst_tensor = tl_tensor_create_slice(NULL, src_entry->tensor, axis, 1,
                                         src_entry->tensor->dtype);
     dst_entry = ln_tensor_entry_create(dst_name, dst_tensor);
     ln_tensor_table_insert(op_arg->tensor_table, dst_name, dst_entry);
     if (arg_name) {
          arg_tensor = tl_tensor_create_slice(NULL, src_entry->tensor, axis, 1,
                                              src_entry->tensor->dtype);
          arg_entry = ln_tensor_entry_create(arg_name, arg_tensor);
          ln_tensor_table_insert(op_arg->tensor_table, arg_name, arg_entry);
     }

     priv = ln_alloc(sizeof(struct priv_s));
     priv->src = src_entry->tensor;
     priv->dst = dst_entry->tensor;
     priv->arg = arg_name ? arg_entry->tensor : NULL;
     priv->dst_name = dst_name;
     priv->arg_name = arg_name;
     priv->axis = axis;
     op_arg->priv = priv;
}

/*
 * Normally we should only do the calculations here. Operations with memory
 * and such should go in pre_run().
 */
static void maxreduce_run(ln_op_arg *op_arg, ln_error **error)
{
     struct priv_s *priv;

     /* do the real work */
     priv = op_arg->priv;
     tl_tensor_maxreduce(priv->src, priv->dst, priv->arg, priv->axis);
}

/*
 * This function should free all tensor memory pre_run() allocated.
 */
static void maxreduce_post_run(ln_op_arg *op_arg, ln_error **error)
{
     struct priv_s *priv;

     /* free the tensor memory allocated in pre_run() */
     priv = op_arg->priv;
     ln_tensor_table_remove(op_arg->tensor_table, priv->dst_name);
     if (priv->arg_name)
          ln_tensor_table_remove(op_arg->tensor_table, priv->arg_name);
     ln_free(op_arg->priv);
}

static ln_op_arg op_arg_maxreduce = {
     .optype = "maxreduce",
     .mtype_in = LN_MEM_CPU,
     .mtype_out = LN_MEM_CPU,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_maxreduce = {
     .op_arg = &op_arg_maxreduce,
     .pre_run = maxreduce_pre_run,
     .static_run = NULL,
     .run = maxreduce_run,
     .post_run = maxreduce_post_run
};

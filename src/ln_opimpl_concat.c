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
     tl_tensor *src1;
     tl_tensor *src2;
     tl_tensor *dst;
     char      *dst_name;
     int        axis;
};

/* TODO: support concat >2 tensors */
/*
 * This function should do the parameter checking and tensor shape inference.
 */
static void concat_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     char *src1_name, *src2_name, *dst_name;
     ln_tensor_entry *src1_entry, *src2_entry, *dst_entry;
     tl_tensor *dst_tensor;
     ln_param_entry *axis_entry;
     int axis;
     int tensors_n, params_n;
     struct priv_s *priv;

     /* check tensors and parameters */
     tensors_n = ln_tensor_list_length(op_arg->tensors_in);
     ln_opck_tensor_in_len_eq(tensors_n, 2);

     tensors_n = ln_tensor_list_length(op_arg->tensors_out);
     ln_opck_tensor_in_len_eq(tensors_n, 1);

     src1_name = ln_tensor_list_find_name(op_arg->tensors_in, "src1");
     ln_opck_tensor_in_exist(src1_name, "src1");
     src1_entry = ln_tensor_table_find(op_arg->tensor_table, src1_name);
     ln_opck_tensor_defined(src1_entry, src1_name);
     ln_opck_tensor_mtype_eq(src1_entry, LN_MEM_CPU);

     src2_name = ln_tensor_list_find_name(op_arg->tensors_in, "src2");
     ln_opck_tensor_in_exist(src2_name, "src2");
     src2_entry = ln_tensor_table_find(op_arg->tensor_table, src2_name);
     ln_opck_tensor_defined(src2_entry, src2_name);
     ln_opck_tensor_mtype_eq(src2_entry, LN_MEM_CPU);
     ln_opck_tensor_issametype(src1_entry, src2_entry);

     dst_name = ln_tensor_list_find_name(op_arg->tensors_out, "dst");
     ln_opck_tensor_out_exist(dst_name, "dst");
     dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
     ln_opck_tensor_not_defined(dst_entry, dst_name);

     params_n = ln_param_list_length(op_arg->params);
     ln_opck_param_len_eq(params_n, 1);

     axis_entry = ln_param_list_find(op_arg->params, "axis");
     ln_opck_param_exist(axis_entry, "axis");
     ln_opck_param_type(axis_entry, LN_PARAM_NUMBER);
     axis = axis_entry->value_int;
     ln_opck_param_satisfy_msg(axis >= 0 && axis < src1_entry->tensor->ndim,
                               "\"axis\" should match the dimensions of \"src1\" and \"src2\"");

     for (int i = 0; i < src1_entry->tensor->ndim; i++) {
          if (i == axis)
               continue;
          ln_opck_tensor_satisfy_msg(src1_entry->tensor->dims[i] == src2_entry->tensor->dims[i],
                                          "\"src1\" and \"src2\" should have the same shape, except in the dimension corresponding to \"axis\"");
     }

     /* define output tensor shape, tensor data should be NULL */
     int *dims;
     dims = ln_clone(src1_entry->tensor->dims,
                     tl_size_of(src1_entry->tensor->dtype));
     dims[axis] = src1_entry->tensor->dims[axis] + src2_entry->tensor->dims[axis];
     dst_tensor = tl_tensor_create(NULL, src1_entry->tensor->ndim, dims,
                                   src1_entry->tensor->dtype);
     dst_entry = ln_tensor_entry_create(dst_name, dst_tensor);
     dst_entry->mtype = LN_MEM_CPU;
     ln_tensor_table_insert(op_arg->tensor_table, dst_name, dst_entry);
     ln_free(dims);

     /* use op_arg->priv to store private data to be used in other functions */
     priv = ln_alloc(sizeof(struct priv_s));
     priv->axis = axis;
     priv->dst = dst_tensor;
     priv->dst_name = dst_name;
     priv->src1 = src1_entry->tensor;
     priv->src2 = src2_entry->tensor;
     op_arg->priv = priv;
}

/*
 * This function should only do the calculations.
 */
static void concat_run(ln_op_arg *op_arg, ln_error **error)
{
     /* TODO: add concat_run */
}

/*
 * This function should free all the memory allocated by other *_run()s.
 */
static void concat_post_run(ln_op_arg *op_arg, ln_error **error)
{
     struct priv_s *priv;

     priv = op_arg->priv;
     ln_tensor_table_remove(op_arg->tensor_table, priv->dst_name);
     ln_free(op_arg->priv);
}

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_concat = {
     .optype = "concat",
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_concat = {
     .op_arg = &op_arg_concat,
     .pre_run = concat_pre_run,
     .static_run = NULL,
     .run = concat_run,
     .post_run = concat_post_run
};

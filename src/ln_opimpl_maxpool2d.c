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
     int       *size;
     int       *stride;
     int       *padding;
};

/*
 * This function should do the parameter checking and tensor shape inference.
 */
static void maxpool2d_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     char *src_name, *dst_name;
     ln_tensor_entry *src_entry, *dst_entry;
     ln_param_entry *size_entry, *stride_entry, *padding_entry;
     int tensors_n, params_n;
     tl_tensor *dst_tensor;
     int       *size;
     int       *stride;
     int       *padding;
     struct priv_s *priv;

     /* check tensors and parameters */
     tensors_n = ln_tensor_list_length(op_arg->tensors_in);
     ln_opck_tensors_in_len_eq(tensors_n, 1);

     tensors_n = ln_tensor_list_length(op_arg->tensors_out);
     ln_opck_tensors_out_len_eq(tensors_n, 1);

     src_name = ln_tensor_list_find_name(op_arg->tensors_in, "src");
     ln_opck_tensor_in_exist(src_name, "src");
     src_entry = ln_tensor_table_find(op_arg->tensor_table, src_name);
     ln_opck_tensor_defined(src_entry, src_name);
     ln_opck_tensor_mtype_eq(src_entry, LN_MEM_CPU);
     ln_opck_tensor_satisfy_msg(src_entry->tensor->ndim == 4,
                                    "`src` should be a 4-dimensional tensor");

     dst_name = ln_tensor_list_find_name(op_arg->tensors_out, "dst");
     ln_opck_tensor_out_exist(dst_name, "dst");
     dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
     ln_opck_tensor_not_defined(dst_entry, dst_name);

     params_n = ln_param_list_length(op_arg->params);
     ln_opck_params_len_eq(params_n, 3);

     size_entry = ln_param_list_find(op_arg->params, "size");
     ln_opck_param_exist(size_entry, "size");
     ln_opck_param_type(size_entry, LN_PARAM_ARRAY_NUMBER);
     ln_opck_param_array_len_eq(size_entry, 2);
     size = size_entry->value_array_int;

     stride_entry = ln_param_list_find(op_arg->params, "stride");
     ln_opck_param_exist(stride_entry, "stride");
     ln_opck_param_type(stride_entry, LN_PARAM_ARRAY_NUMBER);
     ln_opck_param_array_len_eq(stride_entry, 2);
     stride = stride_entry->value_array_int;

     padding_entry = ln_param_list_find(op_arg->params, "padding");
     ln_opck_param_exist(padding_entry, "padding");
     ln_opck_param_type(padding_entry, LN_PARAM_ARRAY_NUMBER);
     ln_opck_param_array_len_eq(padding_entry, 4);
     padding = padding_entry->value_array_int;

     /* define output tensor shape, tensor data should be NULL */
     int dims[4];
     dims[0] = src_entry->tensor->dims[0];
     dims[1] = src_entry->tensor->dims[1];
     dims[2] = ln_compute_output_dim(src_entry->tensor->dims[2], size[0],
                                     stride[0], padding[0] + padding[1]);
     dims[3] = ln_compute_output_dim(src_entry->tensor->dims[3], size[1],
                                     stride[1], padding[2] + padding[3]);
     dst_tensor = tl_tensor_create(NULL, 4, dims, src_entry->tensor->dtype);
     dst_entry = ln_tensor_entry_create(dst_name, dst_tensor);
     dst_entry->mtype = LN_MEM_CUDA;
     ln_tensor_table_insert(op_arg->tensor_table, dst_name, dst_entry);

     /* use op_arg->priv to store private data to be used in other functions */
     priv = ln_alloc(sizeof(struct priv_s));
     priv->dst = dst_tensor;
     priv->dst_name = dst_name;
     priv->padding = padding;
     priv->size = size;
     priv->src = src_entry->tensor;
     priv->stride = stride;
     op_arg->priv = priv;
}

/*
 * This function should only do the calculations.
 */
static void maxpool2d_run(ln_op_arg *op_arg, ln_error **error)
{
     /* TODO: add maxpool2d_run */
}

/*
 * This function should undo everything done by pre_run().
 */
static void maxpool2d_post_run(ln_op_arg *op_arg, ln_error **error)
{
     struct priv_s *priv;

     priv = op_arg->priv;
     ln_tensor_table_remove(op_arg->tensor_table, priv->dst_name);
     ln_free(op_arg->priv);
}

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_maxpool2d = {
     .optype = "maxpool2d",
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_maxpool2d = {
     .op_arg = &op_arg_maxpool2d,
     .pre_run = maxpool2d_pre_run,
     .static_run = NULL,
     .run = maxpool2d_run,
     .post_run = maxpool2d_post_run
};

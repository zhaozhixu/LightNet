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

/* This function should do the parameter checking and tensor shape inference. */
static void slice_pre_run(ln_op_arg *op_arg, ln_error **error)
{
    char                 *src_name;
    ln_tensor_list_entry *src_list_entry;
    ln_tensor_entry      *src_entry;
    tl_tensor            *src;
    char                 *dst_name;
    ln_tensor_list_entry *dst_list_entry;
    ln_tensor_entry      *dst_entry;
    tl_tensor            *dst;
    int                   dst_ndim;
    int                  *dst_dims;
    tl_dtype              dst_dtype;
    ln_param_entry       *axis_entry;
    int                   axis;
    ln_param_entry       *start_entry;
    int                   start;
    ln_param_entry       *len_entry;
    int                   len;
    int                   tensors_in_n;
    int                   tensors_out_n;
    int                   params_n;
    struct priv_s        *priv;

    /* check tensors and parameters */
    tensors_in_n = ln_tensor_list_length(op_arg->tensors_in);
    ln_opck_tensors_in_len_eq(tensors_in_n, 1);

    src_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "src");
    ln_opck_tensor_in_exist(src_list_entry, "src");
    src_name = src_list_entry->name;
    src_entry = ln_tensor_table_find(op_arg->tensor_table, src_name);
    ln_opck_tensor_defined(src_entry, src_name);
    src = src_entry->tensor;
    ln_opck_tensor_mtype_eq(src_entry, LN_MEM_NONE);

    tensors_out_n = ln_tensor_list_length(op_arg->tensors_out);
    ln_opck_tensors_out_len_eq(tensors_out_n, 1);

    dst_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "dst");
    ln_opck_tensor_out_exist(dst_list_entry, "dst");
    dst_name = dst_list_entry->name;
    dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
    ln_opck_tensor_not_defined(dst_entry, dst_name);

    params_n = ln_param_list_length(op_arg->params);
    ln_opck_params_len_eq(params_n, 3);

    axis_entry = ln_param_list_find(op_arg->params, "axis");
    ln_opck_param_exist(axis_entry, "axis");
    ln_opck_param_type(axis_entry, LN_PARAM_NUMBER);
    axis = axis_entry->value_int;
    ln_opck_param_int_ge(axis_entry, 0);
    ln_opck_param_int_lt(axis_entry, src->ndim);

    start_entry = ln_param_list_find(op_arg->params, "start");
    ln_opck_param_exist(start_entry, "start");
    ln_opck_param_type(start_entry, LN_PARAM_NUMBER);
    start = start_entry->value_int;
    ln_opck_param_int_ge(start_entry, 0);
    ln_opck_param_int_lt(start_entry, src->dims[axis]);

    len_entry = ln_param_list_find(op_arg->params, "len");
    ln_opck_param_exist(len_entry, "len");
    ln_opck_param_type(len_entry, LN_PARAM_NUMBER);
    len = len_entry->value_int;
    ln_opck_param_int_gt(len_entry, 0);
    ln_opck_param_int_le(len_entry, src->dims[axis]);
    ln_opck_param_satisfy(len + start <= src->dims[axis]);

    /* define output tensor shape, tensor data should be NULL */
    dst_ndim = src->ndim;
    dst_dtype = src->dtype;
    {
        dst_dims = ln_clone(src->dims, sizeof(int)*src->ndim);
        dst_dims[axis] = len;
    }
    dst = tl_tensor_create(NULL, dst_ndim, dst_dims, dst_dtype);
    dst_entry = ln_tensor_entry_create(dst_name, dst);
    ln_tensor_entry_set_creater(dst_entry, op_arg->name);
    dst_entry->mtype = LN_MEM_NONE;
    ln_tensor_table_insert(op_arg->tensor_table, dst_entry);
    {
        ln_free(dst_dims);
    }

    /* use op_arg->priv to store private data to be used in other functions */
    priv = ln_alloc(sizeof(struct priv_s));
    priv->src = src;
    priv->dst = dst;
    priv->dst_name = dst_name;
    priv->axis = axis;
    priv->start = start;
    priv->len = len;
    op_arg->priv = priv;
}

/* This function should free all the memory allocated by other *_run()s. */
static void slice_post_run(ln_op_arg *op_arg, ln_error **error)
{
    struct priv_s *priv = op_arg->priv;

    ln_tensor_table_remove(op_arg->tensor_table, priv->dst_name);
    ln_free(op_arg->priv);
}

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_slice = {
    .optype = "slice",
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_slice = {
    .op_arg = &op_arg_slice,
    .pre_run = slice_pre_run,
    .static_run = NULL,
    .run = NULL,
    .post_run = slice_post_run
};

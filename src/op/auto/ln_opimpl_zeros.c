/*
 * Copyright (c) 2019 Zhao Zhixu
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
#include "ln_arch.h"

struct priv_s {
    ln_tensor_entry *dst_entry;
    ln_param_entry  *dtype_entry;
    ln_param_entry  *dims_entry;
};

/* This function should do the parameter checking and tensor shape inference. */
static void zeros_pre_run(ln_op_arg *op_arg)
{
    char                 *dst_name;
    ln_tensor_list_entry *dst_list_entry;
    ln_tensor_entry      *dst_entry;
    tl_tensor            *dst;
    int                   dst_ndim;
    int                  *dst_dims;
    tl_dtype              dst_dtype;
    int                   dtype;
    ln_param_entry       *dtype_entry;
    int                  *dims;
    ln_param_entry       *dims_entry;
    int                   tensors_in_n;
    int                   tensors_out_n;
    int                   params_n;
    struct priv_s        *priv;

    /* check tensors and parameters */
    tensors_in_n = ln_tensor_list_length(op_arg->tensors_in);
    ln_opck_tensors_in_len_eq(tensors_in_n, 0);

    tensors_out_n = ln_tensor_list_length(op_arg->tensors_out);
    ln_opck_tensors_out_len_eq(tensors_out_n, 1);

    dst_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "dst");
    ln_opck_tensor_out_exist(dst_list_entry, "dst");
    dst_name = dst_list_entry->name;
    dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
    ln_opck_tensor_not_defined(dst_entry, dst_name);

    params_n = ln_param_list_length(op_arg->params);
    ln_opck_params_len_eq(params_n, 2);

    dtype_entry = ln_param_list_find(op_arg->params, "dtype");
    ln_opck_param_exist(dtype_entry, "dtype");
    ln_opck_param_type(dtype_entry, LN_PARAM_STRING);
    dtype = tl_dtype_from_str(dtype_entry->value_string);
    dtype_entry->value_int = dtype;
    dtype = dtype;
    ln_opck_satisfy_msg(dtype != -1, "`dtype` param should be a supported tl_dtype");

    dims_entry = ln_param_list_find(op_arg->params, "dims");
    ln_opck_param_exist(dims_entry, "dims");
    ln_opck_param_type(dims_entry, LN_PARAM_ARRAY_NUMBER);
    dims = dims_entry->value_array_int;
    ln_opck_param_array_int_gt(dims_entry, 0);
    dims = dims;

    /* define output tensor shape, tensor data should be NULL */
    dst_ndim = dims_entry->array_len;
    dst_dims = dims;
    dst_dtype = dtype;
    dst = tl_tensor_create(NULL, dst_ndim, dst_dims, dst_dtype);
    dst_entry = ln_tensor_entry_create(dst_name, dst);
    dst_entry->offset = dst_list_entry->offset;
    ln_tensor_entry_set_creater(dst_entry, op_arg->name);
    dst_entry->mtype = LN_MEM_NONE;
    ln_tensor_table_insert(op_arg->tensor_table, dst_entry);

    /* use op_arg->priv to store private data to be used in other functions */
    priv = ln_alloc(sizeof(struct priv_s));
    priv->dst_entry = dst_entry;
    priv->dtype_entry = dtype_entry;
    priv->dims_entry = dims_entry;
    op_arg->priv = priv;
}

/* This function should free all the memory allocated by other *_run()s. */
static void zeros_post_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;

    ln_tensor_table_remove(op_arg->tensor_table, priv->dst_entry->name);
    ln_free(priv);
}

static const char *in_arg_names[] = {
    NULL
};

static const char *out_arg_names[] = {
    "dst",
    NULL
};

static const char *param_arg_names[] = {
    "dtype",
    "dims",
    NULL
};

static const ln_param_type param_ptypes[] = {
    LN_PARAM_STRING,
    LN_PARAM_ARRAY_NUMBER,
};

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_zeros = {
    .optype = "zeros",
    .arch = "none",
    .in_arg_names = in_arg_names,
    .out_arg_names = out_arg_names,
    .param_arg_names = param_arg_names,
    .param_ptypes = param_ptypes,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_zeros = {
    .op_arg = &op_arg_zeros,
    .pre_run = zeros_pre_run,
    .static_run = NULL,
    .run = NULL,
    .post_run = zeros_post_run
};

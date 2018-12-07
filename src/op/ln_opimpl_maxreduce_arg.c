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
    tl_tensor *arg;
    char      *arg_name;
    int        axis;
};

/* This function should do the parameter checking and tensor shape inference. */
static void maxreduce_arg_pre_run(ln_op_arg *op_arg, ln_error **error)
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
    char                 *arg_name;
    ln_tensor_list_entry *arg_list_entry;
    ln_tensor_entry      *arg_entry;
    tl_tensor            *arg;
    int                   arg_ndim;
    int                  *arg_dims;
    tl_dtype              arg_dtype;
    ln_param_entry       *axis_entry;
    int                   axis;
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
    ln_opck_tensors_out_len_eq(tensors_out_n, 2);

    dst_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "dst");
    ln_opck_tensor_out_exist(dst_list_entry, "dst");
    dst_name = dst_list_entry->name;
    dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
    ln_opck_tensor_not_defined(dst_entry, dst_name);

    arg_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "arg");
    ln_opck_tensor_out_exist(arg_list_entry, "arg");
    arg_name = arg_list_entry->name;
    arg_entry = ln_tensor_table_find(op_arg->tensor_table, arg_name);
    ln_opck_tensor_not_defined(arg_entry, arg_name);

    params_n = ln_param_list_length(op_arg->params);
    ln_opck_params_len_eq(params_n, 1);

    axis_entry = ln_param_list_find(op_arg->params, "axis");
    ln_opck_param_exist(axis_entry, "axis");
    ln_opck_param_type(axis_entry, LN_PARAM_NUMBER);
    axis = axis_entry->value_int;
    ln_opck_param_satisfy_msg(axis >= 0 && axis < src->ndim, "`axis` should match the dimensions of `src`");

    /* define output tensor shape, tensor data should be NULL */
    dst_ndim = src->ndim;
    dst_dtype = src->dtype;
    {
        dst_dims = ln_clone(src->dims, sizeof(int)*src->ndim);
        dst_dims[axis] = 1;
    }
    dst = tl_tensor_create(NULL, dst_ndim, dst_dims, dst_dtype);
    dst_entry = ln_tensor_entry_create(dst_name, dst);
    ln_tensor_entry_set_creater(dst_entry, op_arg->name);
    dst_entry->mtype = LN_MEM_NONE;
    ln_tensor_table_insert(op_arg->tensor_table, dst_entry);
    {
        ln_free(dst_dims);
    }

    arg_ndim = src->ndim;
    arg_dtype = src->dtype;
    {
        arg_dims = ln_clone(src->dims, sizeof(int)*src->ndim);
        arg_dims[axis] = 1;
    }
    arg = tl_tensor_create(NULL, arg_ndim, arg_dims, arg_dtype);
    arg_entry = ln_tensor_entry_create(arg_name, arg);
    ln_tensor_entry_set_creater(arg_entry, op_arg->name);
    arg_entry->mtype = LN_MEM_NONE;
    ln_tensor_table_insert(op_arg->tensor_table, arg_entry);
    {
        ln_free(arg_dims);
    }

    /* use op_arg->priv to store private data to be used in other functions */
    priv = ln_alloc(sizeof(struct priv_s));
    priv->src = src;
    priv->dst = dst;
    priv->dst_name = dst_name;
    priv->arg = arg;
    priv->arg_name = arg_name;
    priv->axis = axis;
    op_arg->priv = priv;
}

/* This function should free all the memory allocated by other *_run()s. */
static void maxreduce_arg_post_run(ln_op_arg *op_arg, ln_error **error)
{
    struct priv_s *priv = op_arg->priv;

    ln_tensor_table_remove(op_arg->tensor_table, priv->dst_name);
    ln_tensor_table_remove(op_arg->tensor_table, priv->arg_name);
    ln_free(op_arg->priv);
}

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_maxreduce_arg = {
    .optype = "maxreduce_arg",
};

static const char *in_arg_names[] = {
    "src",
    NULL
};

static const char *out_arg_names[] = {
    "dst",
    "arg",
    NULL
};

static const char *param_arg_names[] = {
    "axis",
    NULL
};

static ln_op_info op_info_maxreduce_arg = {
    .in_arg_names = in_arg_names,
    .out_arg_names = out_arg_names,
    .param_arg_names = param_arg_names,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_maxreduce_arg = {
    .op_arg = &op_arg_maxreduce_arg,
    .op_info = &op_info_maxreduce_arg,
    .pre_run = maxreduce_arg_pre_run,
    .static_run = NULL,
    .run = NULL,
    .post_run = maxreduce_arg_post_run
};

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
    ln_tensor_entry *src_entry;
    ln_tensor_entry *scale_entry;
    ln_tensor_entry *offset_entry;
    ln_tensor_entry *mean_entry;
    ln_tensor_entry *var_entry;
    ln_tensor_entry *dst_entry;
    ln_param_entry  *epsilon_entry;
};

/* This function should do the parameter checking and tensor shape inference. */
static void batchnorm_pre_run(ln_op_arg *op_arg)
{
    char                 *src_name;
    ln_tensor_list_entry *src_list_entry;
    ln_tensor_entry      *src_entry;
    tl_tensor            *src;
    char                 *scale_name;
    ln_tensor_list_entry *scale_list_entry;
    ln_tensor_entry      *scale_entry;
    tl_tensor            *scale;
    char                 *offset_name;
    ln_tensor_list_entry *offset_list_entry;
    ln_tensor_entry      *offset_entry;
    tl_tensor            *offset;
    char                 *mean_name;
    ln_tensor_list_entry *mean_list_entry;
    ln_tensor_entry      *mean_entry;
    tl_tensor            *mean;
    char                 *var_name;
    ln_tensor_list_entry *var_list_entry;
    ln_tensor_entry      *var_entry;
    tl_tensor            *var;
    char                 *dst_name;
    ln_tensor_list_entry *dst_list_entry;
    ln_tensor_entry      *dst_entry;
    tl_tensor            *dst;
    int                   dst_ndim;
    int                  *dst_dims;
    tl_dtype              dst_dtype;
    float                 epsilon;
    ln_param_entry       *epsilon_entry;
    int                   tensors_in_n;
    int                   tensors_out_n;
    int                   params_n;
    struct priv_s        *priv;

    /* check tensors and parameters */
    tensors_in_n = ln_tensor_list_length(op_arg->tensors_in);
    ln_opck_tensors_in_len_eq(tensors_in_n, 5);

    src_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "src");
    ln_opck_tensor_in_exist(src_list_entry, "src");
    src_name = src_list_entry->name;
    src_entry = ln_tensor_table_find(op_arg->tensor_table, src_name);
    ln_opck_tensor_defined(src_entry, src_name);
    src = src_entry->tensor;
    src = src;
    ln_opck_tensor_ndim(src_entry, 4);

    scale_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "scale");
    ln_opck_tensor_in_exist(scale_list_entry, "scale");
    scale_name = scale_list_entry->name;
    scale_entry = ln_tensor_table_find(op_arg->tensor_table, scale_name);
    ln_opck_tensor_defined(scale_entry, scale_name);
    scale = scale_entry->tensor;
    scale = scale;
    ln_opck_tensor_ndim(scale_entry, 1);
    ln_opck_tensor_len(scale_entry, src->dims[1]);
    ln_opck_tensor_issametype(scale_entry, src_entry);

    offset_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "offset");
    ln_opck_tensor_in_exist(offset_list_entry, "offset");
    offset_name = offset_list_entry->name;
    offset_entry = ln_tensor_table_find(op_arg->tensor_table, offset_name);
    ln_opck_tensor_defined(offset_entry, offset_name);
    offset = offset_entry->tensor;
    offset = offset;
    ln_opck_tensor_ndim(offset_entry, 1);
    ln_opck_tensor_len(offset_entry, src->dims[1]);
    ln_opck_tensor_issametype(offset_entry, src_entry);

    mean_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "mean");
    ln_opck_tensor_in_exist(mean_list_entry, "mean");
    mean_name = mean_list_entry->name;
    mean_entry = ln_tensor_table_find(op_arg->tensor_table, mean_name);
    ln_opck_tensor_defined(mean_entry, mean_name);
    mean = mean_entry->tensor;
    mean = mean;
    ln_opck_tensor_ndim(mean_entry, 1);
    ln_opck_tensor_len(mean_entry, src->dims[1]);
    ln_opck_tensor_issametype(mean_entry, src_entry);

    var_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "var");
    ln_opck_tensor_in_exist(var_list_entry, "var");
    var_name = var_list_entry->name;
    var_entry = ln_tensor_table_find(op_arg->tensor_table, var_name);
    ln_opck_tensor_defined(var_entry, var_name);
    var = var_entry->tensor;
    var = var;
    ln_opck_tensor_ndim(var_entry, 1);
    ln_opck_tensor_len(var_entry, src->dims[1]);
    ln_opck_tensor_issametype(var_entry, src_entry);

    tensors_out_n = ln_tensor_list_length(op_arg->tensors_out);
    ln_opck_tensors_out_len_eq(tensors_out_n, 1);

    dst_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "dst");
    ln_opck_tensor_out_exist(dst_list_entry, "dst");
    dst_name = dst_list_entry->name;
    dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
    ln_opck_tensor_not_defined(dst_entry, dst_name);

    params_n = ln_param_list_length(op_arg->params);
    ln_opck_params_len_eq(params_n, 1);

    epsilon_entry = ln_param_list_find(op_arg->params, "epsilon");
    ln_opck_param_exist(epsilon_entry, "epsilon");
    ln_opck_param_type(epsilon_entry, LN_PARAM_NUMBER);
    epsilon = epsilon_entry->value_float;
    ln_opck_param_float_gt(epsilon_entry, 0);
    epsilon = epsilon;

    /* define output tensor shape, tensor data should be NULL */
    dst_ndim = src->ndim;
    dst_dims = src->dims;
    dst_dtype = src->dtype;
    dst = tl_tensor_create(NULL, dst_ndim, dst_dims, dst_dtype);
    dst_entry = ln_tensor_entry_create(dst_name, dst);
    ln_tensor_entry_set_creater(dst_entry, op_arg->name);
    dst_entry->mtype = LN_MEM_NONE;
    ln_tensor_table_insert(op_arg->tensor_table, dst_entry);

    /* use op_arg->priv to store private data to be used in other functions */
    priv = ln_alloc(sizeof(struct priv_s));
    priv->src_entry = src_entry;
    priv->scale_entry = scale_entry;
    priv->offset_entry = offset_entry;
    priv->mean_entry = mean_entry;
    priv->var_entry = var_entry;
    priv->dst_entry = dst_entry;
    priv->epsilon_entry = epsilon_entry;
    op_arg->priv = priv;
}

/* This function should free all the memory allocated by other *_run()s. */
static void batchnorm_post_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;

    ln_tensor_table_remove(op_arg->tensor_table, priv->dst_entry->name);
    ln_free(priv);
}

static const char *in_arg_names[] = {
    "src",
    "scale",
    "offset",
    "mean",
    "var",
    NULL
};

static const char *out_arg_names[] = {
    "dst",
    NULL
};

static const char *param_arg_names[] = {
    "epsilon",
    NULL
};

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_batchnorm = {
    .optype = "batchnorm",
    .arch = "none",
    .in_arg_names = in_arg_names,
    .out_arg_names = out_arg_names,
    .param_arg_names = param_arg_names,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_batchnorm = {
    .op_arg = &op_arg_batchnorm,
    .pre_run = batchnorm_pre_run,
    .static_run = NULL,
    .run = NULL,
    .post_run = batchnorm_post_run
};

/*
 * Copyright (c) 2018-2019 Zhao Zhixu
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
    ln_tensor_entry *src_entry;
    ln_tensor_entry *src_index_entry;
    ln_tensor_entry *dst_entry;
    ln_param_entry  *len_entry;
    ln_param_entry  *stride_entry;
};

/* This function should do the parameter checking and tensor shape inference. */
static void pick1d_cpu_pre_run(ln_op_arg *op_arg)
{
    char                 *src_name;
    ln_tensor_list_entry *src_list_entry;
    ln_tensor_entry      *src_entry;
    tl_tensor            *src;
    char                 *src_index_name;
    ln_tensor_list_entry *src_index_list_entry;
    ln_tensor_entry      *src_index_entry;
    tl_tensor            *src_index;
    char                 *dst_name;
    ln_tensor_list_entry *dst_list_entry;
    ln_tensor_entry      *dst_entry;
    tl_tensor            *dst;
    int                   dst_ndim;
    int                  *dst_dims;
    tl_dtype              dst_dtype;
    int                   len;
    ln_param_entry       *len_entry;
    int                   stride;
    ln_param_entry       *stride_entry;
    int                   tensors_in_n;
    int                   tensors_out_n;
    int                   params_n;
    struct priv_s        *priv;

    /* check tensors and parameters */
    tensors_in_n = ln_tensor_list_length(op_arg->tensors_in);
    ln_opck_tensors_in_len_eq(tensors_in_n, 2);

    src_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "src");
    ln_opck_tensor_in_exist(src_list_entry, "src");
    src_name = src_list_entry->name;
    src_entry = ln_tensor_table_find(op_arg->tensor_table, src_name);
    ln_opck_tensor_defined(src_entry, src_name);
    src = src_entry->tensor;
    src = src;
    ln_opck_tensor_mtype_eq(src_entry, LN_MEM_CPU);
    ln_opck_tensor_ndim(src_entry, 1);

    src_index_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "src_index");
    ln_opck_tensor_in_exist(src_index_list_entry, "src_index");
    src_index_name = src_index_list_entry->name;
    src_index_entry = ln_tensor_table_find(op_arg->tensor_table, src_index_name);
    ln_opck_tensor_defined(src_index_entry, src_index_name);
    src_index = src_index_entry->tensor;
    src_index = src_index;
    ln_opck_tensor_mtype_eq(src_index_entry, LN_MEM_CPU);
    ln_opck_tensor_dtype_eq(src_index_entry, TL_INT32);
    ln_opck_tensor_ndim(src_index_entry, 1);

    tensors_out_n = ln_tensor_list_length(op_arg->tensors_out);
    ln_opck_tensors_out_len_eq(tensors_out_n, 1);

    dst_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "dst");
    ln_opck_tensor_out_exist(dst_list_entry, "dst");
    dst_name = dst_list_entry->name;
    dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
    ln_opck_tensor_not_defined(dst_entry, dst_name);

    params_n = ln_param_list_length(op_arg->params);
    ln_opck_params_len_eq(params_n, 2);

    len_entry = ln_param_list_find(op_arg->params, "len");
    ln_opck_param_exist(len_entry, "len");
    ln_opck_param_type(len_entry, LN_PARAM_NUMBER);
    len = len_entry->value_int;
    ln_opck_param_int_le(len_entry, src_index->len);
    len = len;

    stride_entry = ln_param_list_find(op_arg->params, "stride");
    ln_opck_param_exist(stride_entry, "stride");
    ln_opck_param_type(stride_entry, LN_PARAM_NUMBER);
    stride = stride_entry->value_int;
    ln_opck_param_int_ge(stride_entry, 1);
    stride = stride;

    /* define output tensor shape, tensor data should be NULL */
    dst_ndim = 1;
    dst_dtype = src->dtype;
    {
        dst_dims = ln_alloc(sizeof(int) * 1);
        dst_dims[0] = len * stride;
    }
    dst = tl_tensor_create(NULL, dst_ndim, dst_dims, dst_dtype);
    dst_entry = ln_tensor_entry_create(dst_name, dst);
    dst_entry->offset = dst_list_entry->offset;
    ln_tensor_entry_set_creater(dst_entry, op_arg->name);
    dst_entry->mtype = LN_MEM_CPU;
    ln_tensor_table_insert(op_arg->tensor_table, dst_entry);
    {
        ln_free(dst_dims);
    }

    /* use op_arg->priv to store private data to be used in other functions */
    priv = ln_alloc(sizeof(struct priv_s));
    priv->src_entry = src_entry;
    priv->src_index_entry = src_index_entry;
    priv->dst_entry = dst_entry;
    priv->len_entry = len_entry;
    priv->stride_entry = stride_entry;
    op_arg->priv = priv;
}

/* This function should only do the calculations. */
static void pick1d_cpu_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;

    {
    }
}

/* This function should free all the memory allocated by other *_run()s. */
static void pick1d_cpu_post_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;

    ln_tensor_table_remove(op_arg->tensor_table, priv->dst_entry->name);
    ln_free(priv);
}

static const char *in_arg_names[] = {
    "src",
    "src_index",
    NULL
};

static const char *out_arg_names[] = {
    "dst",
    NULL
};

static const char *param_arg_names[] = {
    "len",
    "stride",
    NULL
};

static const ln_param_type param_ptypes[] = {
    LN_PARAM_NUMBER,
    LN_PARAM_NUMBER,
};

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_pick1d_cpu = {
    .optype = "pick1d_cpu",
    .arch = "cpu",
    .in_arg_names = in_arg_names,
    .out_arg_names = out_arg_names,
    .param_arg_names = param_arg_names,
    .param_ptypes = param_ptypes,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_pick1d_cpu = {
    .op_arg = &op_arg_pick1d_cpu,
    .pre_run = pick1d_cpu_pre_run,
    .static_run = NULL,
    .run = pick1d_cpu_run,
    .post_run = pick1d_cpu_post_run
};

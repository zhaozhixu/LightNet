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
#include "ln_cuda.h"

struct priv_s {
    ln_tensor_entry *src_entry;
    ln_tensor_entry *dst_entry;
    ln_param_entry  *dims_entry;
};

/* This function should do the parameter checking and tensor shape inference. */
static void reshape_cuda_pre_run(ln_op_arg *op_arg)
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
    int                  *dims;
    ln_param_entry       *dims_entry;
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
    src = src;
    ln_opck_tensor_mtype_eq(src_entry, LN_MEM_CUDA);

    tensors_out_n = ln_tensor_list_length(op_arg->tensors_out);
    ln_opck_tensors_out_len_eq(tensors_out_n, 1);

    dst_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "dst");
    ln_opck_tensor_out_exist(dst_list_entry, "dst");
    dst_name = dst_list_entry->name;
    dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
    ln_opck_tensor_not_defined(dst_entry, dst_name);

    params_n = ln_param_list_length(op_arg->params);
    ln_opck_params_len_eq(params_n, 1);

    dims_entry = ln_param_list_find(op_arg->params, "dims");
    ln_opck_param_exist(dims_entry, "dims");
    ln_opck_param_type(dims_entry, LN_PARAM_ARRAY_NUMBER);
    dims = dims_entry->value_array_int;
    ln_opck_param_array_int_ge(dims_entry, 1);
    dims = dims;
    {
        {
            char shape1[LN_MAXLINE];
            char shape2[LN_MAXLINE];
            ln_opck_satisfy_msg(src->len == ln_compute_length(dims_entry->array_len, dims), "`src` (%s) tensor's length %d should be equal to the reshaped (%s) length %d", ln_sprint_shape(shape1, src->ndim, src->dims), src->len, ln_sprint_shape(shape2, dims_entry->array_len, dims), ln_compute_length(dims_entry->array_len, dims));
        }
    }

    /* define output tensor shape, tensor data should be NULL */
    dst_ndim = dims_entry->array_len;
    dst_dims = dims;
    dst_dtype = src->dtype;
    dst = tl_tensor_create(NULL, dst_ndim, dst_dims, dst_dtype);
    dst_entry = ln_tensor_entry_create(dst_name, dst);
    dst_entry->offset = dst_list_entry->offset;
    ln_tensor_entry_set_creater(dst_entry, op_arg->name);
    ln_tensor_entry_set_owner(dst_entry, op_arg->tensor_table, src_name);
    dst_entry->mtype = LN_MEM_CUDA;
    ln_tensor_table_insert(op_arg->tensor_table, dst_entry);

    /* use op_arg->priv to store private data to be used in other functions */
    priv = ln_alloc(sizeof(struct priv_s));
    priv->src_entry = src_entry;
    priv->dst_entry = dst_entry;
    priv->dims_entry = dims_entry;
    op_arg->priv = priv;
}

/* This function runs only once per instance right after memory allocation. */
static void reshape_cuda_static_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;
    tl_tensor     *src = priv->src_entry->tensor;
    tl_tensor     *dst = priv->dst_entry->tensor;

    {
        dst->data = src->data;
    }
}

/* This function should free all the memory allocated by other *_run()s. */
static void reshape_cuda_post_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;

    ln_tensor_table_remove(op_arg->tensor_table, priv->dst_entry->name);
    ln_free(priv);
}

static const char *in_arg_names[] = {
    "src",
    NULL
};

static const char *out_arg_names[] = {
    "dst",
    NULL
};

static const char *param_arg_names[] = {
    "dims",
    NULL
};

static const ln_param_type param_ptypes[] = {
    LN_PARAM_ARRAY_NUMBER,
};

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_reshape_cuda = {
    .optype = "reshape_cuda",
    .arch = "cuda",
    .in_arg_names = in_arg_names,
    .out_arg_names = out_arg_names,
    .param_arg_names = param_arg_names,
    .param_ptypes = param_ptypes,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_reshape_cuda = {
    .op_arg = &op_arg_reshape_cuda,
    .pre_run = reshape_cuda_pre_run,
    .static_run = reshape_cuda_static_run,
    .run = NULL,
    .post_run = reshape_cuda_post_run
};

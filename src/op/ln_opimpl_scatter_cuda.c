/*
 * Copyright (c) 2018-2020 Zhao Zhixu
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
#include "ln_cuda.h"

struct priv_s {
    ln_list *dst_entries;
    ln_tensor_entry *src_entry;
};

/*
 * This function should do the parameter checking and tensor shape inference.
 */
static void scatter_cuda_pre_run(ln_op_arg *op_arg)
{
    ln_tensor_entry *te;
    ln_tensor_list_entry *tle;
    ln_tensor_list_entry *src_list_entry;
    ln_tensor_entry *src_entry;
    tl_tensor *src;
    tl_tensor *dst;
    ln_list *dst_entries;
    int dst_n;
    int dst_ndim;
    int *dst_dims;

    /* check tensors and parameters */
    src_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "src");
    ln_opck_tensor_in_exist(src_list_entry, "src");
    ln_opck_tensor_defined(src_list_entry, src_list_entry->name);
    src_entry = ln_tensor_table_find(op_arg->tensor_table,
                                      src_list_entry->name);
    ln_opck_tensor_mtype_eq(src_entry, LN_MEM_CUDA);
    ln_opck_satisfy(src_entry->tensor->ndim > 1);
    src = src_entry->tensor;

    dst_n = 0;
    dst_entries = NULL;
    dst_ndim = src_entry->tensor->ndim - 1;
    dst_dims = ln_alloc(sizeof(int) * dst_ndim);
    memmove(dst_dims, &src_entry->tensor->dims[1], sizeof(int) * dst_ndim);
    LN_LIST_FOREACH(tle, op_arg->tensors_out) {
        if (!ln_streqn(tle->arg_name, "dst", 3))
            continue;
        te = ln_tensor_table_find(op_arg->tensor_table, tle->name);
        ln_opck_tensor_not_defined(te, tle->name);

        /* define output tensor shape, tensor data should be NULL */
        dst = tl_tensor_create(NULL, dst_ndim, dst_dims, src->dtype);
        te = ln_tensor_entry_create(tle->name, dst);
        te->offset = tle->offset;
        ln_tensor_entry_set_creater(te, op_arg->name);
        te->mtype = LN_MEM_CUDA;
        ln_tensor_table_insert(op_arg->tensor_table, te);
        dst_entries = ln_list_append(dst_entries, te);
        dst_n++;
    }
    ln_free(dst_dims);
    ln_opck_satisfy(dst_n == src_entry->tensor->dims[0]);

    struct priv_s *priv;
    priv = ln_alloc(sizeof(struct priv_s));
    priv->dst_entries = dst_entries;
    priv->src_entry = src_entry;
    op_arg->priv = priv;
}

/* This function should only do the calculations. */
static void scatter_cuda_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;
    ln_tensor_entry *te;
    tl_tensor *src = priv->src_entry->tensor;
    void *src_p = src->data;
    size_t size;

    LN_LIST_FOREACH(te, priv->dst_entries) {
        size = tl_tensor_size(te->tensor);
        ln_memcpy_d2d(te->tensor->data, src_p, size);
        src_p = tl_padd(src_p, size, 1);
    }
}

/*
 * This function should free all the memory allocated by other *_run()s.
 */
static void scatter_cuda_post_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;
    ln_tensor_entry *te;

    LN_LIST_FOREACH(te, priv->dst_entries) {
        ln_tensor_table_remove(op_arg->tensor_table, te->name);
    }
    ln_list_free(priv->dst_entries);
    ln_free(op_arg->priv);
}

static const char *in_arg_names[] = {
    "src",
    NULL
};

static const char *out_arg_names[] = {
    NULL
};

static const char *param_arg_names[] = {
    NULL
};

static const ln_param_type param_ptypes[] = {
};

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_scatter_cuda = {
    .optype = "scatter_cuda",
    .arch = "none",
    .in_arg_names = in_arg_names,
    .out_arg_names = out_arg_names,
    .param_arg_names = param_arg_names,
    .param_ptypes = param_ptypes,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_scatter_cuda = {
    .op_arg = &op_arg_scatter_cuda,
    .pre_run = scatter_cuda_pre_run,
    .static_run = NULL,
    .run = scatter_cuda_run,
    .post_run = scatter_cuda_post_run,
    .calc_offset = NULL,
};

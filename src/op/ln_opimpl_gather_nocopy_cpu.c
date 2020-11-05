/*
 * Copyright (c) 2018-2020 Zhixu Zhao
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
    ln_list *src_entries;
    ln_tensor_entry *dst_entry;
};

/*
 * This function should do the parameter checking and tensor shape inference.
 */
static void gather_nocopy_cpu_pre_run(ln_op_arg *op_arg)
{
    ln_tensor_entry *te;
    ln_tensor_list_entry *tle;
    ln_tensor_list_entry *src1_list_entry;
    ln_tensor_list_entry *dst_list_entry;
    ln_tensor_entry *src1_entry;
    ln_tensor_entry *dst_entry;
    tl_tensor *src1;
    tl_tensor *dst;
    ln_list *src_entries;
    int src_n;

    /* check tensors and parameters */
    src1_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "src1");
    ln_opck_tensor_in_exist(src1_list_entry, "src1");
    ln_opck_tensor_defined(src1_list_entry, src1_list_entry->name);
    src1_entry = ln_tensor_table_find(op_arg->tensor_table,
                                      src1_list_entry->name);
    ln_opck_tensor_mtype_eq(src1_entry, LN_MEM_CPU);
    src1 = src1_entry->tensor;
    src_n = 0;
    src_entries = NULL;
    dst_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "dst");
    LN_LIST_FOREACH(tle, op_arg->tensors_in) {
        if (!ln_streqn(tle->arg_name, "src", 3))
            continue;
        te = ln_tensor_table_find(op_arg->tensor_table, tle->name);
        ln_opck_tensor_defined(te, tle->name);
        ln_opck_tensor_issametype(te, src1_entry);
        ln_opck_tensor_issameshape(te, src1_entry);
        ln_opck_tensor_mtype_eq(te, LN_MEM_CPU);
        src_entries = ln_list_append(src_entries, te);
        ln_tensor_entry_set_owner(te, op_arg->tensor_table, dst_list_entry->name);
        src_n++;
    }

    dst_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "dst");
    ln_opck_tensor_out_exist(dst_list_entry, "dst");
    dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_list_entry->name);
    ln_opck_tensor_not_defined(dst_entry, dst_list_entry->name);

    /* define output tensor shape, tensor data should be NULL */
    int dst_ndim = src1->ndim + 1;
    int *dst_dims = ln_alloc(sizeof(int) * dst_ndim);
    dst_dims[0] = src_n;
    memmove(&dst_dims[1], src1->dims, sizeof(int) * src1->ndim);
    dst = tl_tensor_create(NULL, dst_ndim, dst_dims, src1->dtype);
    dst_entry = ln_tensor_entry_create(dst_list_entry->name, dst);
    dst_entry->offset = dst_list_entry->offset;
    ln_tensor_entry_set_creater(dst_entry, op_arg->name);
    dst_entry->mtype = LN_MEM_CPU;
    ln_tensor_table_insert(op_arg->tensor_table, dst_entry);
    ln_free(dst_dims);

    struct priv_s *priv;
    priv = ln_alloc(sizeof(struct priv_s));
    priv->src_entries = src_entries;
    priv->dst_entry = dst_entry;
    op_arg->priv = priv;
}

/*
 * This function should free all the memory allocated by other *_run()s.
 */
static void gather_nocopy_cpu_post_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;

    ln_tensor_table_remove(op_arg->tensor_table, priv->dst_entry->name);
    ln_list_free(priv->src_entries);
    ln_free(op_arg->priv);
}

static size_t gather_nocopy_cpu_calc_offset(ln_op_arg *op_arg,
                                            ln_tensor_entry *te)
{
    struct priv_s *priv = op_arg->priv;
    ln_tensor_entry *dst_entry = priv->dst_entry;
    ln_tensor_entry *entry;
    size_t offset = dst_entry->offset;
    size_t size;

    LN_LIST_FOREACH(entry, priv->src_entries) {
        if (ln_streq(entry->name, te->name))
            return offset;
        size = tl_tensor_size(entry->tensor);
        offset += size;
    }
    return 0;
}

static const char *in_arg_names[] = {
    NULL
};

static const char *out_arg_names[] = {
    "dst",
    NULL
};

static const char *param_arg_names[] = {
    NULL
};

static const ln_param_type param_ptypes[] = {
};

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_gather_nocopy_cpu = {
    .optype = "gather_nocopy_cpu",
    .arch = "cpu",
    .in_arg_names = in_arg_names,
    .out_arg_names = out_arg_names,
    .param_arg_names = param_arg_names,
    .param_ptypes = param_ptypes,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_gather_nocopy_cpu = {
    .op_arg = &op_arg_gather_nocopy_cpu,
    .pre_run = gather_nocopy_cpu_pre_run,
    .static_run = NULL,
    .run = NULL,
    .post_run = gather_nocopy_cpu_post_run,
    .calc_offset = gather_nocopy_cpu_calc_offset,
};

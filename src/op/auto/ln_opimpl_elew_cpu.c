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
    ln_tensor_entry *src1_entry;
    ln_tensor_entry *src2_entry;
    ln_tensor_entry *dst_entry;
    ln_param_entry  *elew_op_entry;
};

/* This function should do the parameter checking and tensor shape inference. */
static void elew_cpu_pre_run(ln_op_arg *op_arg)
{
    char                 *src1_name;
    ln_tensor_list_entry *src1_list_entry;
    ln_tensor_entry      *src1_entry;
    tl_tensor            *src1;
    char                 *src2_name;
    ln_tensor_list_entry *src2_list_entry;
    ln_tensor_entry      *src2_entry;
    tl_tensor            *src2;
    char                 *dst_name;
    ln_tensor_list_entry *dst_list_entry;
    ln_tensor_entry      *dst_entry;
    tl_tensor            *dst;
    int                   dst_ndim;
    int                  *dst_dims;
    tl_dtype              dst_dtype;
    int                   elew_op;
    ln_param_entry       *elew_op_entry;
    int                   tensors_in_n;
    int                   tensors_out_n;
    int                   params_n;
    struct priv_s        *priv;

    /* check tensors and parameters */
    tensors_in_n = ln_tensor_list_length(op_arg->tensors_in);
    ln_opck_tensors_in_len_eq(tensors_in_n, 2);

    src1_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "src1");
    ln_opck_tensor_in_exist(src1_list_entry, "src1");
    src1_name = src1_list_entry->name;
    src1_entry = ln_tensor_table_find(op_arg->tensor_table, src1_name);
    ln_opck_tensor_defined(src1_entry, src1_name);
    src1 = src1_entry->tensor;
    src1 = src1;
    ln_opck_tensor_mtype_eq(src1_entry, LN_MEM_CPU);

    src2_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "src2");
    ln_opck_tensor_in_exist(src2_list_entry, "src2");
    src2_name = src2_list_entry->name;
    src2_entry = ln_tensor_table_find(op_arg->tensor_table, src2_name);
    ln_opck_tensor_defined(src2_entry, src2_name);
    src2 = src2_entry->tensor;
    src2 = src2;
    ln_opck_tensor_mtype_eq(src2_entry, LN_MEM_CPU);
    ln_opck_tensor_issametype(src2_entry, src1_entry);
    ln_opck_tensor_issameshape(src2_entry, src1_entry);

    tensors_out_n = ln_tensor_list_length(op_arg->tensors_out);
    ln_opck_tensors_out_len_eq(tensors_out_n, 1);

    dst_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "dst");
    ln_opck_tensor_out_exist(dst_list_entry, "dst");
    dst_name = dst_list_entry->name;
    dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
    ln_opck_tensor_not_defined(dst_entry, dst_name);

    params_n = ln_param_list_length(op_arg->params);
    ln_opck_params_len_eq(params_n, 1);

    elew_op_entry = ln_param_list_find(op_arg->params, "elew_op");
    ln_opck_param_exist(elew_op_entry, "elew_op");
    ln_opck_param_type(elew_op_entry, LN_PARAM_STRING);
    elew_op = tl_elew_op_from_str(elew_op_entry->value_string);
    elew_op_entry->value_int = elew_op;
    elew_op = elew_op;
    ln_opck_satisfy_msg(elew_op != -1, "`elew_op` param should be a supported tl_elew_op");

    /* define output tensor shape, tensor data should be NULL */
    dst_ndim = src1->ndim;
    dst_dims = src1->dims;
    dst_dtype = src1->dtype;
    dst = tl_tensor_create(NULL, dst_ndim, dst_dims, dst_dtype);
    dst_entry = ln_tensor_entry_create(dst_name, dst);
    ln_tensor_entry_set_creater(dst_entry, op_arg->name);
    dst_entry->mtype = LN_MEM_CPU;
    ln_tensor_table_insert(op_arg->tensor_table, dst_entry);

    /* use op_arg->priv to store private data to be used in other functions */
    priv = ln_alloc(sizeof(struct priv_s));
    priv->src1_entry = src1_entry;
    priv->src2_entry = src2_entry;
    priv->dst_entry = dst_entry;
    priv->elew_op_entry = elew_op_entry;
    op_arg->priv = priv;
}

/* This function should only do the calculations. */
static void elew_cpu_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;
    tl_tensor     *src1 = priv->src1_entry->tensor;
    tl_tensor     *src2 = priv->src2_entry->tensor;
    tl_tensor     *dst = priv->dst_entry->tensor;
    int            elew_op = priv->elew_op_entry->value_int;

    {
        tl_tensor_elew(src1, src2, dst, elew_op);
    }
}

/* This function should free all the memory allocated by other *_run()s. */
static void elew_cpu_post_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;

    ln_tensor_table_remove(op_arg->tensor_table, priv->dst_entry->name);
    ln_free(priv);
}

static const char *in_arg_names[] = {
    "src1",
    "src2",
    NULL
};

static const char *out_arg_names[] = {
    "dst",
    NULL
};

static const char *param_arg_names[] = {
    "elew_op",
    NULL
};

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_elew_cpu = {
    .optype = "elew_cpu",
    .arch = "cpu",
    .in_arg_names = in_arg_names,
    .out_arg_names = out_arg_names,
    .param_arg_names = param_arg_names,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_elew_cpu = {
    .op_arg = &op_arg_elew_cpu,
    .pre_run = elew_cpu_pre_run,
    .static_run = NULL,
    .run = elew_cpu_run,
    .post_run = elew_cpu_post_run
};

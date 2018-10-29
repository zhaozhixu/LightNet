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
     tl_tensor  *src1;
     tl_tensor  *src2;
     tl_tensor  *dst;
     char       *dst_name;
     tl_elew_op  elew_op;
};

/*
 * This function should do the parameter checking and tensor shape inference.
 */
static void elew_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     char *src1_name, *src2_name, *dst_name;
     ln_tensor_entry *src1_entry, *src2_entry, *dst_entry;
     ln_param_entry *elew_op_entry;
     int tensors_n, params_n;
     tl_elew_op elew_op;
     tl_tensor *dst_tensor;
     struct priv_s *priv;

     /* check tensors and parameters */
     tensors_n = ln_tensor_list_length(op_arg->tensors_in);
     ln_opck_tensor_in_len_eq(tensors_n, 2);

     tensors_n = ln_tensor_list_length(op_arg->tensors_out);
     ln_opck_tensor_out_len_eq(tensors_n, 1);

     src1_name = ln_tensor_list_find_name(op_arg->tensors_in, "src1");
     ln_opck_tensor_in_exist(src1_name, "src1");
     src1_entry = ln_tensor_table_find(op_arg->tensor_table, src1_name);
     ln_opck_tensor_defined(src1_entry, src1_name);
     ln_opck_tensor_mtype_eq(src1_entry, LN_MEM_CPU);

     src2_name = ln_tensor_list_find_name(op_arg->tensors_in, "src2");
     ln_opck_tensor_in_exist(src2_name, "src2");
     src2_entry = ln_tensor_table_find(op_arg->tensor_table, src2_name);
     ln_opck_tensor_defined(src2_entry, src2_name);
     ln_opck_tensor_mtype_eq(src2_entry, LN_MEM_CPU);
     ln_opck_tensor_issameshape(src1_entry, src2_entry);
     ln_opck_tensor_issametype(src1_entry, src2_entry);

     dst_name = ln_tensor_list_find_name(op_arg->tensors_out, "dst");
     ln_opck_tensor_out_exist(dst_name, "dst");
     dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
     ln_opck_tensor_not_defined(dst_entry, dst_name);

     params_n = ln_param_list_length(op_arg->params);
     ln_opck_param_len_eq(params_n, 1);

     elew_op_entry = ln_param_list_find(op_arg->params, "elew_op");
     ln_opck_param_exist(elew_op_entry, "elew_op");
     ln_opck_param_type(elew_op_entry, LN_PARAM_STRING);

     elew_op = tl_elew_op_from_str(elew_op_entry->value_string);
     ln_opck_param_satisfy_msg(elew_op != -1,
                                   "\"elew_op\" param should be a supported tl_elew_op");

     /* define output tensor shape, tensor data should be NULL */
     dst_tensor = tl_tensor_create(NULL, src1_entry->tensor->ndim,
                                   src2_entry->tensor->dims,
                                   src1_entry->tensor->dtype);
     dst_entry = ln_tensor_entry_create(dst_name, dst_tensor);
     dst_entry->mtype = LN_MEM_CPU;
     ln_tensor_table_insert(op_arg->tensor_table, dst_name, dst_entry);

     /* use op_arg->priv to store private data to be used in other functions */
     priv = ln_alloc(sizeof(struct priv_s));
     priv->src1 = src1_entry->tensor;
     priv->src2 = src2_entry->tensor;
     priv->dst = dst_tensor;
     priv->dst_name = dst_name;
     priv->elew_op = elew_op;
     op_arg->priv = priv;
}

/*
 * This function should only do the calculations.
 */
static void elew_run(ln_op_arg *op_arg, ln_error **error)
{
     struct priv_s *priv;

     priv = op_arg->priv;
     tl_tensor_elew(priv->src1, priv->src2, priv->dst, priv->elew_op);
}

/*
 * This function should undo everything done by pre_run().
 */
static void elew_post_run(ln_op_arg *op_arg, ln_error **error)
{
     struct priv_s *priv;

     priv = op_arg->priv;
     ln_tensor_table_remove(op_arg->tensor_table, priv->dst_name);
     ln_free(op_arg->priv);
}

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_elew = {
     .optype = "elew",
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_elew = {
     .op_arg = &op_arg_elew,
     .pre_run = elew_pre_run,
     .static_run = NULL,
     .run = elew_run,
     .post_run = elew_post_run
};

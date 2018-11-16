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
     tl_tensor *scale;
     tl_tensor *offset;
     tl_tensor *mean;
     tl_tensor *var;
     char      *dst_name;
     double     epsilon;
};

/*
 * This function should do the parameter checking and tensor shape inference.
 */
static void batchnorm_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     char *src_name, *dst_name, *scale_name, *offset_name, *mean_name, *var_name;
     ln_tensor_entry *src_entry, *dst_entry, *scale_entry, *offset_entry, *mean_entry, *var_entry;
     tl_tensor *dst_tensor;
     ln_param_entry *epsilon_entry;
     int tensors_n, params_n;
     double epsilon;
     struct priv_s *priv;

     /* check tensors and parameters */
     tensors_n = ln_tensor_list_length(op_arg->tensors_in);
     ln_opck_tensors_in_len_eq(tensors_n, 5);

     tensors_n = ln_tensor_list_length(op_arg->tensors_out);
     ln_opck_tensors_out_len_eq(tensors_n, 1);

     src_name = ln_tensor_list_find_name(op_arg->tensors_in, "src");
     ln_opck_tensor_in_exist(src_name, "src");
     src_entry = ln_tensor_table_find(op_arg->tensor_table, src_name);
     ln_opck_tensor_defined(src_entry, src_name);
     ln_opck_tensor_mtype_eq(src_entry, LN_MEM_CPU);
     ln_opck_tensor_satisfy_msg(src_entry->tensor->ndim == 4,
                                "`src` should be a 4-dimensional tensor");

     scale_name = ln_tensor_list_find_name(op_arg->tensors_in, "scale");
     ln_opck_tensor_in_exist(scale_name, "scale");
     scale_entry = ln_tensor_table_find(op_arg->tensor_table, scale_name);
     ln_opck_tensor_defined(scale_entry, scale_name);
     ln_opck_tensor_mtype_eq(scale_entry, LN_MEM_CPU);
     ln_opck_tensor_issametype(scale_entry, src_entry);
     ln_opck_tensor_satisfy_msg(scale_entry->tensor->ndim == 1
                                && scale_entry->tensor->len == src_entry->tensor->dims[1],
                                "`scale` should be a 1-dimensional tensor of size equal to the number of channels of `src`");

     offset_name = ln_tensor_list_find_name(op_arg->tensors_in, "offset");
     ln_opck_tensor_in_exist(offset_name, "offset");
     offset_entry = ln_tensor_table_find(op_arg->tensor_table, offset_name);
     ln_opck_tensor_defined(offset_entry, offset_name);
     ln_opck_tensor_mtype_eq(offset_entry, LN_MEM_CPU);
     ln_opck_tensor_issametype(offset_entry, src_entry);
     ln_opck_tensor_satisfy_msg(offset_entry->tensor->ndim == 1
                                && offset_entry->tensor->len == src_entry->tensor->dims[1],
                                "`offset` should be a 1-dimensional tensor of size equal to the number of channels of `src`");

     mean_name = ln_tensor_list_find_name(op_arg->tensors_in, "mean");
     ln_opck_tensor_in_exist(mean_name, "mean");
     mean_entry = ln_tensor_table_find(op_arg->tensor_table, mean_name);
     ln_opck_tensor_defined(mean_entry, mean_name);
     ln_opck_tensor_mtype_eq(mean_entry, LN_MEM_CPU);
     ln_opck_tensor_issametype(mean_entry, src_entry);
     ln_opck_tensor_satisfy_msg(mean_entry->tensor->ndim == 1
                                && mean_entry->tensor->len == src_entry->tensor->dims[1],
                                "`mean` should be a 1-dimensional tensor of size equal to the number of channels of `src`");

     var_name = ln_tensor_list_find_name(op_arg->tensors_in, "var");
     ln_opck_tensor_in_exist(var_name, "var");
     var_entry = ln_tensor_table_find(op_arg->tensor_table, var_name);
     ln_opck_tensor_defined(var_entry, var_name);
     ln_opck_tensor_mtype_eq(var_entry, LN_MEM_CPU);
     ln_opck_tensor_issametype(var_entry, src_entry);
     ln_opck_tensor_satisfy_msg(var_entry->tensor->ndim == 1
                                && var_entry->tensor->len == src_entry->tensor->dims[1],
                                "`var` should be a 1-dimensional tensor of size equal to the number of channels of `src`");

     dst_name = ln_tensor_list_find_name(op_arg->tensors_out, "dst");
     ln_opck_tensor_out_exist(dst_name, "dst");
     dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
     ln_opck_tensor_not_defined(dst_entry, dst_name);

     params_n = ln_param_list_length(op_arg->params);
     ln_opck_params_len_eq(params_n, 1);

     epsilon_entry = ln_param_list_find(op_arg->params, "epsilon");
     ln_opck_param_exist(epsilon_entry, "epsilon");
     ln_opck_param_type(epsilon_entry, LN_PARAM_NUMBER);
     epsilon = epsilon_entry->value_double;
     ln_opck_param_satisfy_msg(epsilon > 0, "`epsilon` should be above zero");

     /* define output tensor shape, tensor data should be NULL */
     dst_tensor = tl_tensor_create(NULL, src_entry->tensor->ndim,
                                   src_entry->tensor->dims, src_entry->tensor->dtype);
     dst_entry = ln_tensor_entry_create(dst_name, dst_tensor);
     ln_tensor_entry_set_creater(dst_entry, op_arg->name);
     dst_entry->mtype = LN_MEM_CPU;
     ln_tensor_table_insert(op_arg->tensor_table, dst_name, dst_entry);

     /* use op_arg->priv to store private data to be used in other functions */
     priv = ln_alloc(sizeof(struct priv_s));
     priv->dst = dst_tensor;
     priv->dst_name = dst_name;
     priv->epsilon = epsilon;
     priv->mean = mean_entry->tensor;
     priv->offset = offset_entry->tensor;
     priv->scale = scale_entry->tensor;
     priv->src = src_entry->tensor;
     priv->var = var_entry->tensor;
     op_arg->priv = priv;
}

/*
 * This function should only do the calculations.
 */
static void batchnorm_run(ln_op_arg *op_arg, ln_error **error)
{
     /* TODO: add batchnorm_run */
}

/*
 * This function should free all the memory allocated by other *_run()s.
 */
static void batchnorm_post_run(ln_op_arg *op_arg, ln_error **error)
{
     struct priv_s *priv;

     priv = op_arg->priv;
     ln_tensor_table_remove(op_arg->tensor_table, priv->dst_name);
     ln_free(op_arg->priv);
}

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_batchnorm = {
     .optype = "batchnorm",
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_batchnorm = {
     .op_arg = &op_arg_batchnorm,
     .pre_run = batchnorm_pre_run,
     .static_run = NULL,
     .run = batchnorm_run,
     .post_run = batchnorm_post_run
};

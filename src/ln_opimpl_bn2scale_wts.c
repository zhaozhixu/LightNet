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
    tl_tensor *src_mean;
    tl_tensor *src_var;
    tl_tensor *src_scale;
    tl_tensor *src_offset;
    tl_tensor *dst_scale;
    char      *dst_scale_name;
    tl_tensor *dst_shift;
    char      *dst_shift_name;
    tl_tensor *dst_power;
    char      *dst_power_name;
    float      epsilon;
};

/* This function should do the parameter checking and tensor shape inference. */
static void bn2scale_wts_pre_run(ln_op_arg *op_arg, ln_error **error)
{
    char            *src_mean_name;
    ln_tensor_entry *src_mean_entry;
    tl_tensor       *src_mean;
    char            *src_var_name;
    ln_tensor_entry *src_var_entry;
    tl_tensor       *src_var;
    char            *src_scale_name;
    ln_tensor_entry *src_scale_entry;
    tl_tensor       *src_scale;
    char            *src_offset_name;
    ln_tensor_entry *src_offset_entry;
    tl_tensor       *src_offset;
    char            *dst_scale_name;
    ln_tensor_entry *dst_scale_entry;
    tl_tensor       *dst_scale;
    int              dst_scale_ndim;
    int             *dst_scale_dims;
    tl_dtype         dst_scale_dtype;
    char            *dst_shift_name;
    ln_tensor_entry *dst_shift_entry;
    tl_tensor       *dst_shift;
    int              dst_shift_ndim;
    int             *dst_shift_dims;
    tl_dtype         dst_shift_dtype;
    char            *dst_power_name;
    ln_tensor_entry *dst_power_entry;
    tl_tensor       *dst_power;
    int              dst_power_ndim;
    int             *dst_power_dims;
    tl_dtype         dst_power_dtype;
    ln_param_entry  *epsilon_entry;
    float            epsilon;
    int              tensors_in_n;
    int              tensors_out_n;
    int              params_n;
    struct priv_s   *priv;

    /* check tensors and parameters */
    tensors_in_n = ln_tensor_list_length(op_arg->tensors_in);
    ln_opck_tensors_in_len_eq(tensors_in_n, 4);

    src_mean_name = ln_tensor_list_find_name(op_arg->tensors_in, "src_mean");
    ln_opck_tensor_in_exist(src_mean_name, "src_mean");
    src_mean_entry = ln_tensor_table_find(op_arg->tensor_table, src_mean_name);
    ln_opck_tensor_defined(src_mean_entry, src_mean_name);
    src_mean = src_mean_entry->tensor;
    ln_opck_tensor_mtype_eq(src_mean_entry, LN_MEM_CPU);
    ln_opck_tensor_dtype_eq(src_mean_entry, TL_FLOAT);
    ln_opck_tensor_isstatic(src_mean_entry);
    ln_opck_tensor_satisfy_msg(src_mean->ndim == 1, "`src_mean` should have only 1 dimension");

    src_var_name = ln_tensor_list_find_name(op_arg->tensors_in, "src_var");
    ln_opck_tensor_in_exist(src_var_name, "src_var");
    src_var_entry = ln_tensor_table_find(op_arg->tensor_table, src_var_name);
    ln_opck_tensor_defined(src_var_entry, src_var_name);
    src_var = src_var_entry->tensor;
    ln_opck_tensor_mtype_eq(src_var_entry, LN_MEM_CPU);
    ln_opck_tensor_dtype_eq(src_var_entry, TL_FLOAT);
    ln_opck_tensor_issameshape(src_var_entry, src_mean_entry);
    ln_opck_tensor_isstatic(src_var_entry);

    src_scale_name = ln_tensor_list_find_name(op_arg->tensors_in, "src_scale");
    ln_opck_tensor_in_exist(src_scale_name, "src_scale");
    src_scale_entry = ln_tensor_table_find(op_arg->tensor_table, src_scale_name);
    ln_opck_tensor_defined(src_scale_entry, src_scale_name);
    src_scale = src_scale_entry->tensor;
    ln_opck_tensor_mtype_eq(src_scale_entry, LN_MEM_CPU);
    ln_opck_tensor_dtype_eq(src_scale_entry, TL_FLOAT);
    ln_opck_tensor_issameshape(src_scale_entry, src_mean_entry);
    ln_opck_tensor_isstatic(src_scale_entry);

    src_offset_name = ln_tensor_list_find_name(op_arg->tensors_in, "src_offset");
    ln_opck_tensor_in_exist(src_offset_name, "src_offset");
    src_offset_entry = ln_tensor_table_find(op_arg->tensor_table, src_offset_name);
    ln_opck_tensor_defined(src_offset_entry, src_offset_name);
    src_offset = src_offset_entry->tensor;
    ln_opck_tensor_mtype_eq(src_offset_entry, LN_MEM_CPU);
    ln_opck_tensor_dtype_eq(src_offset_entry, TL_FLOAT);
    ln_opck_tensor_issameshape(src_offset_entry, src_mean_entry);
    ln_opck_tensor_isstatic(src_offset_entry);

    tensors_out_n = ln_tensor_list_length(op_arg->tensors_out);
    ln_opck_tensors_out_len_eq(tensors_out_n, 3);

    dst_scale_name = ln_tensor_list_find_name(op_arg->tensors_out, "dst_scale");
    ln_opck_tensor_in_exist(dst_scale_name, "dst_scale");
    dst_scale_entry = ln_tensor_table_find(op_arg->tensor_table, dst_scale_name);
    ln_opck_tensor_not_defined(dst_scale_entry, dst_scale_name);

    dst_shift_name = ln_tensor_list_find_name(op_arg->tensors_out, "dst_shift");
    ln_opck_tensor_in_exist(dst_shift_name, "dst_shift");
    dst_shift_entry = ln_tensor_table_find(op_arg->tensor_table, dst_shift_name);
    ln_opck_tensor_not_defined(dst_shift_entry, dst_shift_name);

    dst_power_name = ln_tensor_list_find_name(op_arg->tensors_out, "dst_power");
    ln_opck_tensor_in_exist(dst_power_name, "dst_power");
    dst_power_entry = ln_tensor_table_find(op_arg->tensor_table, dst_power_name);
    ln_opck_tensor_not_defined(dst_power_entry, dst_power_name);

    params_n = ln_param_list_length(op_arg->params);
    ln_opck_params_len_eq(params_n, 1);

    epsilon_entry = ln_param_list_find(op_arg->params, "epsilon");
    ln_opck_param_exist(epsilon_entry, "epsilon");
    ln_opck_param_type(epsilon_entry, LN_PARAM_NUMBER);
    epsilon = epsilon_entry->value_float;
    ln_opck_param_satisfy_msg(epsilon > 0, "`epsilon` should be positive");

    /* define output tensor shape, tensor data should be NULL */
    dst_scale_ndim = 1;
    dst_scale_dims = src_mean->dims;
    dst_scale_dtype = src_mean->dtype;
    dst_scale = tl_tensor_create(NULL, dst_scale_ndim, dst_scale_dims, dst_scale_dtype);
    dst_scale_entry = ln_tensor_entry_create(dst_scale_name, dst_scale);
    ln_tensor_entry_set_creater(dst_scale_entry, op_arg->name);
    dst_scale_entry->isstatic = 1;
    dst_scale_entry->mtype = LN_MEM_CPU;
    ln_tensor_table_insert(op_arg->tensor_table, dst_scale_name, dst_scale_entry);

    dst_shift_ndim = 1;
    dst_shift_dims = src_mean->dims;
    dst_shift_dtype = src_mean->dtype;
    dst_shift = tl_tensor_create(NULL, dst_shift_ndim, dst_shift_dims, dst_shift_dtype);
    dst_shift_entry = ln_tensor_entry_create(dst_shift_name, dst_shift);
    ln_tensor_entry_set_creater(dst_shift_entry, op_arg->name);
    dst_shift_entry->isstatic = 1;
    dst_shift_entry->mtype = LN_MEM_CPU;
    ln_tensor_table_insert(op_arg->tensor_table, dst_shift_name, dst_shift_entry);

    dst_power_ndim = 1;
    dst_power_dims = src_mean->dims;
    dst_power_dtype = src_mean->dtype;
    dst_power = tl_tensor_create(NULL, dst_power_ndim, dst_power_dims, dst_power_dtype);
    dst_power_entry = ln_tensor_entry_create(dst_power_name, dst_power);
    ln_tensor_entry_set_creater(dst_power_entry, op_arg->name);
    dst_power_entry->isstatic = 1;
    dst_power_entry->mtype = LN_MEM_CPU;
    ln_tensor_table_insert(op_arg->tensor_table, dst_power_name, dst_power_entry);

    /* use op_arg->priv to store private data to be used in other functions */
    priv = ln_alloc(sizeof(struct priv_s));
    priv->src_mean = src_mean;
    priv->src_var = src_var;
    priv->src_scale = src_scale;
    priv->src_offset = src_offset;
    priv->dst_scale = dst_scale;
    priv->dst_scale_name = dst_scale_name;
    priv->dst_shift = dst_shift;
    priv->dst_shift_name = dst_shift_name;
    priv->dst_power = dst_power;
    priv->dst_power_name = dst_power_name;
    priv->epsilon = epsilon;
    op_arg->priv = priv;
}

/* This function blocks only once per instance right after memory allocation. */
static void bn2scale_wts_static_run(ln_op_arg *op_arg, ln_error **error)
{
    struct priv_s *priv = op_arg->priv;

    {
        for (int i = 0; i < priv->dst_scale->len; i++) {
            ((float *)priv->dst_scale->data)[i] = ((float *)priv->src_scale->data)[i] / (((float *)priv->src_var->data)[i] + priv->epsilon);
            ((float *)priv->dst_shift->data)[i] = ((float *)priv->src_offset->data)[i] - ((float *)priv->src_mean->data)[i] * ((float *)priv->src_scale->data)[i] / (((float *)priv->src_var->data)[i] + priv->epsilon);
            ((float *)priv->dst_power->data)[i] = 1;
        }
    }
}

/* This function should free all the memory allocated by other *_run()s. */
static void bn2scale_wts_post_run(ln_op_arg *op_arg, ln_error **error)
{
    struct priv_s *priv = op_arg->priv;

    ln_tensor_table_remove(op_arg->tensor_table, priv->dst_scale_name);
    ln_tensor_table_remove(op_arg->tensor_table, priv->dst_shift_name);
    ln_tensor_table_remove(op_arg->tensor_table, priv->dst_power_name);
    ln_free(op_arg->priv);
}

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_bn2scale_wts = {
    .optype = "bn2scale_wts",
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_bn2scale_wts = {
    .op_arg = &op_arg_bn2scale_wts,
    .pre_run = bn2scale_wts_pre_run,
    .static_run = bn2scale_wts_static_run,
    .run = NULL,
    .post_run = bn2scale_wts_post_run
};

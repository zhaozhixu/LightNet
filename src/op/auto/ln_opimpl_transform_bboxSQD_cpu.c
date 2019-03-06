/*
 * Copyright (c) 2019 Zhao Zhixu
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
    ln_tensor_entry *src_delta_entry;
    ln_tensor_entry *src_anchor_entry;
    ln_tensor_entry *dst_entry;
    ln_param_entry  *width_entry;
    ln_param_entry  *height_entry;
    ln_param_entry  *img_width_entry;
    ln_param_entry  *img_height_entry;
    ln_param_entry  *x_shift_entry;
    ln_param_entry  *y_shift_entry;
};

/* This function should do the parameter checking and tensor shape inference. */
static void transform_bboxSQD_cpu_pre_run(ln_op_arg *op_arg)
{
    char                 *src_delta_name;
    ln_tensor_list_entry *src_delta_list_entry;
    ln_tensor_entry      *src_delta_entry;
    tl_tensor            *src_delta;
    char                 *src_anchor_name;
    ln_tensor_list_entry *src_anchor_list_entry;
    ln_tensor_entry      *src_anchor_entry;
    tl_tensor            *src_anchor;
    char                 *dst_name;
    ln_tensor_list_entry *dst_list_entry;
    ln_tensor_entry      *dst_entry;
    tl_tensor            *dst;
    int                   dst_ndim;
    int                  *dst_dims;
    tl_dtype              dst_dtype;
    int                   width;
    ln_param_entry       *width_entry;
    int                   height;
    ln_param_entry       *height_entry;
    int                   img_width;
    ln_param_entry       *img_width_entry;
    int                   img_height;
    ln_param_entry       *img_height_entry;
    int                   x_shift;
    ln_param_entry       *x_shift_entry;
    int                   y_shift;
    ln_param_entry       *y_shift_entry;
    int                   tensors_in_n;
    int                   tensors_out_n;
    int                   params_n;
    struct priv_s        *priv;

    /* check tensors and parameters */
    tensors_in_n = ln_tensor_list_length(op_arg->tensors_in);
    ln_opck_tensors_in_len_eq(tensors_in_n, 2);

    src_delta_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "src_delta");
    ln_opck_tensor_in_exist(src_delta_list_entry, "src_delta");
    src_delta_name = src_delta_list_entry->name;
    src_delta_entry = ln_tensor_table_find(op_arg->tensor_table, src_delta_name);
    ln_opck_tensor_defined(src_delta_entry, src_delta_name);
    src_delta = src_delta_entry->tensor;
    src_delta = src_delta;
    ln_opck_tensor_mtype_eq(src_delta_entry, LN_MEM_CPU);
    ln_opck_tensor_dtype_eq(src_delta_entry, TL_FLOAT);
    ln_opck_tensor_ndim(src_delta_entry, 5);
    ln_opck_satisfy(src_delta->dims[4] == 4);

    src_anchor_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "src_anchor");
    ln_opck_tensor_in_exist(src_anchor_list_entry, "src_anchor");
    src_anchor_name = src_anchor_list_entry->name;
    src_anchor_entry = ln_tensor_table_find(op_arg->tensor_table, src_anchor_name);
    ln_opck_tensor_defined(src_anchor_entry, src_anchor_name);
    src_anchor = src_anchor_entry->tensor;
    src_anchor = src_anchor;
    ln_opck_tensor_mtype_eq(src_anchor_entry, LN_MEM_CPU);
    ln_opck_tensor_issametype(src_anchor_entry, src_delta_entry);
    ln_opck_tensor_issameshape(src_anchor_entry, src_delta_entry);

    tensors_out_n = ln_tensor_list_length(op_arg->tensors_out);
    ln_opck_tensors_out_len_eq(tensors_out_n, 1);

    dst_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "dst");
    ln_opck_tensor_out_exist(dst_list_entry, "dst");
    dst_name = dst_list_entry->name;
    dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
    ln_opck_tensor_not_defined(dst_entry, dst_name);

    params_n = ln_param_list_length(op_arg->params);
    ln_opck_params_len_eq(params_n, 6);

    width_entry = ln_param_list_find(op_arg->params, "width");
    ln_opck_param_exist(width_entry, "width");
    ln_opck_param_type(width_entry, LN_PARAM_NUMBER);
    width = width_entry->value_int;
    ln_opck_param_int_gt(width_entry, 0);
    width = width;

    height_entry = ln_param_list_find(op_arg->params, "height");
    ln_opck_param_exist(height_entry, "height");
    ln_opck_param_type(height_entry, LN_PARAM_NUMBER);
    height = height_entry->value_int;
    ln_opck_param_int_gt(height_entry, 0);
    height = height;

    img_width_entry = ln_param_list_find(op_arg->params, "img_width");
    ln_opck_param_exist(img_width_entry, "img_width");
    ln_opck_param_type(img_width_entry, LN_PARAM_NUMBER);
    img_width = img_width_entry->value_int;
    ln_opck_param_int_gt(img_width_entry, 0);
    img_width = img_width;

    img_height_entry = ln_param_list_find(op_arg->params, "img_height");
    ln_opck_param_exist(img_height_entry, "img_height");
    ln_opck_param_type(img_height_entry, LN_PARAM_NUMBER);
    img_height = img_height_entry->value_int;
    ln_opck_param_int_gt(img_height_entry, 0);
    img_height = img_height;

    x_shift_entry = ln_param_list_find(op_arg->params, "x_shift");
    ln_opck_param_exist(x_shift_entry, "x_shift");
    ln_opck_param_type(x_shift_entry, LN_PARAM_NUMBER);
    x_shift = x_shift_entry->value_int;
    x_shift = x_shift;

    y_shift_entry = ln_param_list_find(op_arg->params, "y_shift");
    ln_opck_param_exist(y_shift_entry, "y_shift");
    ln_opck_param_type(y_shift_entry, LN_PARAM_NUMBER);
    y_shift = y_shift_entry->value_int;
    y_shift = y_shift;

    /* define output tensor shape, tensor data should be NULL */
    dst_ndim = src_delta->ndim;
    dst_dims = src_delta->dims;
    dst_dtype = src_delta->dtype;
    dst = tl_tensor_create(NULL, dst_ndim, dst_dims, dst_dtype);
    dst_entry = ln_tensor_entry_create(dst_name, dst);
    dst_entry->offset = dst_list_entry->offset;
    ln_tensor_entry_set_creater(dst_entry, op_arg->name);
    dst_entry->mtype = LN_MEM_CPU;
    ln_tensor_table_insert(op_arg->tensor_table, dst_entry);

    /* use op_arg->priv to store private data to be used in other functions */
    priv = ln_alloc(sizeof(struct priv_s));
    priv->src_delta_entry = src_delta_entry;
    priv->src_anchor_entry = src_anchor_entry;
    priv->dst_entry = dst_entry;
    priv->width_entry = width_entry;
    priv->height_entry = height_entry;
    priv->img_width_entry = img_width_entry;
    priv->img_height_entry = img_height_entry;
    priv->x_shift_entry = x_shift_entry;
    priv->y_shift_entry = y_shift_entry;
    op_arg->priv = priv;
}

/* This function should only do the calculations. */
static void transform_bboxSQD_cpu_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;

    {
    }
}

/* This function should free all the memory allocated by other *_run()s. */
static void transform_bboxSQD_cpu_post_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;

    ln_tensor_table_remove(op_arg->tensor_table, priv->dst_entry->name);
    ln_free(priv);
}

static const char *in_arg_names[] = {
    "src_delta",
    "src_anchor",
    NULL
};

static const char *out_arg_names[] = {
    "dst",
    NULL
};

static const char *param_arg_names[] = {
    "width",
    "height",
    "img_width",
    "img_height",
    "x_shift",
    "y_shift",
    NULL
};

static const ln_param_type param_ptypes[] = {
    LN_PARAM_NUMBER,
    LN_PARAM_NUMBER,
    LN_PARAM_NUMBER,
    LN_PARAM_NUMBER,
    LN_PARAM_NUMBER,
    LN_PARAM_NUMBER,
};

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_transform_bboxSQD_cpu = {
    .optype = "transform_bboxSQD_cpu",
    .arch = "cpu",
    .in_arg_names = in_arg_names,
    .out_arg_names = out_arg_names,
    .param_arg_names = param_arg_names,
    .param_ptypes = param_ptypes,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_transform_bboxSQD_cpu = {
    .op_arg = &op_arg_transform_bboxSQD_cpu,
    .pre_run = transform_bboxSQD_cpu_pre_run,
    .static_run = NULL,
    .run = transform_bboxSQD_cpu_run,
    .post_run = transform_bboxSQD_cpu_post_run
};

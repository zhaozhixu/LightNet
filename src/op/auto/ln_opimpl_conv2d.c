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

/* NOTE: this file is automatically generated by protos/op/conv2d.op
   using tools/addop.pl */

#include <assert.h>
#include "ln_op.h"
#include "ln_arch.h"

struct priv_s {
    ln_tensor_entry *src_entry;
    ln_tensor_entry *weight_entry;
    ln_tensor_entry *bias_entry;
    ln_tensor_entry *dst_entry;
    ln_param_entry  *group_entry;
    ln_param_entry  *size_entry;
    ln_param_entry  *stride_entry;
    ln_param_entry  *dilation_entry;
    ln_param_entry  *padding_entry;
    ln_param_entry  *autopad_entry;
};

/* This function should do the parameter checking and tensor shape inference. */
static void conv2d_pre_run(ln_op_arg *op_arg)
{
    char                 *src_name;
    ln_tensor_list_entry *src_list_entry;
    ln_tensor_entry      *src_entry;
    tl_tensor            *src;
    char                 *weight_name;
    ln_tensor_list_entry *weight_list_entry;
    ln_tensor_entry      *weight_entry;
    tl_tensor            *weight;
    char                 *bias_name;
    ln_tensor_list_entry *bias_list_entry;
    ln_tensor_entry      *bias_entry;
    tl_tensor            *bias;
    char                 *dst_name;
    ln_tensor_list_entry *dst_list_entry;
    ln_tensor_entry      *dst_entry;
    tl_tensor            *dst;
    int                   dst_ndim;
    int                  *dst_dims;
    tl_dtype              dst_dtype;
    int                   group;
    ln_param_entry       *group_entry;
    int                  *size;
    ln_param_entry       *size_entry;
    int                  *stride;
    ln_param_entry       *stride_entry;
    int                  *dilation;
    ln_param_entry       *dilation_entry;
    int                  *padding;
    ln_param_entry       *padding_entry;
    char                 *autopad;
    ln_param_entry       *autopad_entry;
    int                   tensors_in_n;
    int                   tensors_out_n;
    int                   params_n;
    struct priv_s        *priv;

    /* check tensors and parameters */
    tensors_in_n = ln_tensor_list_length(op_arg->tensors_in);
    ln_opck_tensors_in_len_eq(tensors_in_n, 3);

    src_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "src");
    ln_opck_tensor_in_exist(src_list_entry, "src");
    src_name = src_list_entry->name;
    src_entry = ln_tensor_table_find(op_arg->tensor_table, src_name);
    ln_opck_tensor_defined(src_entry, src_name);
    src = src_entry->tensor;
    src = src;
    ln_opck_tensor_ndim(src_entry, 4);

    weight_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "weight");
    ln_opck_tensor_in_exist(weight_list_entry, "weight");
    weight_name = weight_list_entry->name;
    weight_entry = ln_tensor_table_find(op_arg->tensor_table, weight_name);
    ln_opck_tensor_defined(weight_entry, weight_name);
    weight = weight_entry->tensor;
    weight = weight;
    ln_opck_tensor_ndim(weight_entry, 4);

    bias_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "bias");
    ln_opck_tensor_in_exist(bias_list_entry, "bias");
    bias_name = bias_list_entry->name;
    bias_entry = ln_tensor_table_find(op_arg->tensor_table, bias_name);
    ln_opck_tensor_defined(bias_entry, bias_name);
    bias = bias_entry->tensor;
    bias = bias;
    ln_opck_tensor_ndim(bias_entry, 1);
    /* begin custom code */
    {
        char shape1[LN_MAXLINE];
        char shape2[LN_MAXLINE];
        ln_opck_satisfy_msg(bias->dims[0] == weight->dims[0], "'bias' (%s) should have the size of dims[0] of 'weight' (%s)", ln_sprint_shape(shape1, bias->ndim, bias->dims), ln_sprint_shape(shape2, weight->ndim, weight->dims));
    }
    /* end custom code */

    tensors_out_n = ln_tensor_list_length(op_arg->tensors_out);
    ln_opck_tensors_out_len_eq(tensors_out_n, 1);

    dst_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "dst");
    ln_opck_tensor_out_exist(dst_list_entry, "dst");
    dst_name = dst_list_entry->name;
    dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
    ln_opck_tensor_not_defined(dst_entry, dst_name);

    params_n = ln_param_list_length(op_arg->params);
    ln_opck_params_len_eq(params_n, 6);

    group_entry = ln_param_list_find(op_arg->params, "group");
    ln_opck_param_exist(group_entry, "group");
    ln_opck_param_type(group_entry, LN_PARAM_NUMBER);
    group = group_entry->value_int;
    ln_opck_param_int_ge(group_entry, 1);
    group = group;
    /* begin custom code */
    {
        char shape1[LN_MAXLINE];
        char shape2[LN_MAXLINE];
        ln_opck_satisfy_msg(weight->dims[1]*group == src->dims[1], "'weight' (%s)'s dims[1] multiplies group (%d) should be equal to the dims[1] of 'src' (%s)", ln_sprint_shape(shape1, weight->ndim, weight->dims), group, ln_sprint_shape(shape2, src->ndim, src->dims));
    }
    /* end custom code */

    size_entry = ln_param_list_find(op_arg->params, "size");
    ln_opck_param_exist(size_entry, "size");
    ln_opck_param_type(size_entry, LN_PARAM_ARRAY_NUMBER);
    ln_opck_param_array_len_eq(size_entry, 2);
    size = size_entry->value_array_int;
    ln_opck_param_array_int_ge(size_entry, 1);
    size = size;
    /* begin custom code */
    {
        char shape1[LN_MAXLINE];
        char shape2[LN_MAXLINE];
        ln_opck_satisfy_msg(size[0] == weight->dims[2] && size[1] == weight->dims[3], "'size' (%s) should be equal to the last two dimensions of 'weight' (%s)", ln_sprint_shape(shape1, size_entry->array_len, size), ln_sprint_shape(shape2, weight->ndim, weight->dims));
    }
    /* end custom code */

    stride_entry = ln_param_list_find(op_arg->params, "stride");
    ln_opck_param_exist(stride_entry, "stride");
    ln_opck_param_type(stride_entry, LN_PARAM_ARRAY_NUMBER);
    ln_opck_param_array_len_eq(stride_entry, 2);
    stride = stride_entry->value_array_int;
    ln_opck_param_array_int_ge(stride_entry, 1);
    stride = stride;

    dilation_entry = ln_param_list_find(op_arg->params, "dilation");
    ln_opck_param_exist(dilation_entry, "dilation");
    ln_opck_param_type(dilation_entry, LN_PARAM_ARRAY_NUMBER);
    ln_opck_param_array_len_eq(dilation_entry, 2);
    dilation = dilation_entry->value_array_int;
    ln_opck_param_array_int_ge(dilation_entry, 1);
    dilation = dilation;

    padding_entry = ln_param_list_find(op_arg->params, "padding");
    ln_opck_param_exist(padding_entry, "padding");
    ln_opck_param_type(padding_entry, LN_PARAM_ARRAY_NUMBER);
    ln_opck_param_array_len_eq(padding_entry, 4);
    padding = padding_entry->value_array_int;
    ln_opck_param_array_int_ge(padding_entry, 0);
    padding = padding;

    autopad_entry = ln_param_list_find(op_arg->params, "autopad");
    ln_opck_param_exist(autopad_entry, "autopad");
    ln_opck_param_type(autopad_entry, LN_PARAM_STRING);
    autopad = autopad_entry->value_string;
    autopad = autopad;
    /* begin custom code */
    {
    if (ln_streq(autopad, "VALID") || ln_streq(autopad, "SAME_UPPER") ||
        ln_streq(autopad, "SAME_LOWER")) {
        ln_autopadding_conv(padding, &src->dims[2], size, stride, dilation, 2, autopad);
    } else if (ln_streq(autopad, "NOTSET")){
    } else {
        ln_msg_warn("unsupported 'autopad' %s", autopad);
    }
    }
    /* end custom code */

    /* define output tensor shape, tensor data should be NULL */
    dst_ndim = src->ndim;
    dst_dtype = src->dtype;
    /* begin custom code */
    {
    dst_dims = ln_alloc(sizeof(int)*4);
    dst_dims[0] = src->dims[0];
    dst_dims[1] = weight->dims[0];
    dst_dims[2] = ln_output_dim_conv(src->dims[2], size[0], stride[0], padding[0] + padding[2], dilation[0]);
    dst_dims[3] = ln_output_dim_conv(src->dims[3], size[1], stride[1], padding[1] + padding[3], dilation[1]);
    }
    /* end custom code */
    dst = tl_tensor_create(NULL, dst_ndim, dst_dims, dst_dtype);
    dst_entry = ln_tensor_entry_create(dst_name, dst);
    dst_entry->offset = dst_list_entry->offset;
    ln_tensor_entry_set_creater(dst_entry, op_arg->name);
    dst_entry->mtype = LN_MEM_NONE;
    ln_tensor_table_insert(op_arg->tensor_table, dst_entry);
    /* begin custom code */
    ln_free(dst_dims);
    /* end custom code */

    /* use op_arg->priv to store private data to be used in other functions */
    priv = ln_alloc(sizeof(struct priv_s));
    priv->src_entry = src_entry;
    priv->weight_entry = weight_entry;
    priv->bias_entry = bias_entry;
    priv->dst_entry = dst_entry;
    priv->group_entry = group_entry;
    priv->size_entry = size_entry;
    priv->stride_entry = stride_entry;
    priv->dilation_entry = dilation_entry;
    priv->padding_entry = padding_entry;
    priv->autopad_entry = autopad_entry;
    op_arg->priv = priv;
}

/* This function should free all the memory allocated by other *_run()s. */
static void conv2d_post_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;

    ln_tensor_table_remove(op_arg->tensor_table, priv->dst_entry->name);
    ln_free(priv);
}

static const char *in_arg_names[] = {
    "src",
    "weight",
    "bias",
    NULL
};

static const char *out_arg_names[] = {
    "dst",
    NULL
};

static const char *param_arg_names[] = {
    "group",
    "size",
    "stride",
    "dilation",
    "padding",
    "autopad",
    NULL
};

static const ln_param_type param_ptypes[] = {
    LN_PARAM_NUMBER,
    LN_PARAM_ARRAY_NUMBER,
    LN_PARAM_ARRAY_NUMBER,
    LN_PARAM_ARRAY_NUMBER,
    LN_PARAM_ARRAY_NUMBER,
    LN_PARAM_STRING,
};

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_conv2d = {
    .optype = "conv2d",
    .arch = "none",
    .in_arg_names = in_arg_names,
    .out_arg_names = out_arg_names,
    .param_arg_names = param_arg_names,
    .param_ptypes = param_ptypes,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_conv2d = {
    .op_arg = &op_arg_conv2d,
    .pre_run = conv2d_pre_run,
    .static_run = NULL,
    .run = NULL,
    .post_run = conv2d_post_run,
    .calc_offset = NULL,
};

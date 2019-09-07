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

#include "ln_arch.h"

extern ln_op ln_opimpl_create_cuda;
extern ln_op ln_opimpl_conv2d_cuda;
extern ln_op ln_opimpl_maxpool2d_cuda;
extern ln_op ln_opimpl_maxreduce_cuda;
extern ln_op ln_opimpl_relu_cuda;
extern ln_op ln_opimpl_reshape_cuda;
extern ln_op ln_opimpl_slice_cuda;
extern ln_op ln_opimpl_transpose_cuda;
extern ln_op ln_opimpl_zeros_cuda;
extern ln_op ln_opimpl_elew_cuda;
extern ln_op ln_opimpl_softmax_cuda;
extern ln_op ln_opimpl_concat_cuda;
extern ln_op ln_opimpl_batchnorm_cuda;
extern ln_op ln_opimpl_upsample_cuda;
extern ln_op ln_opimpl_maxreduce_arg_cuda;
extern ln_op ln_opimpl_print_cuda;
extern ln_op ln_opimpl_sigmoid_cuda;
extern ln_op ln_opimpl_sort1d_cuda;
extern ln_op ln_opimpl_arange_cuda;
extern ln_op ln_opimpl_sort1d_by_key_cuda;
extern ln_op ln_opimpl_transform_bboxSQD_cuda;
extern ln_op ln_opimpl_rearange_cuda;
extern ln_op ln_opimpl_pick1d_cuda;
extern ln_op ln_opimpl_fprint_cuda;
extern ln_op ln_opimpl_lrelu_cuda;
extern ln_op ln_opimpl_detect_yolov3_cuda;
extern ln_op ln_opimpl_avgpool2d_cuda;
extern ln_op ln_opimpl_resize_cuda;
extern ln_op ln_opimpl_gather_cuda;
extern ln_op ln_opimpl_gather_nocopy_cuda;
extern ln_op ln_opimpl_scatter_cuda;
extern ln_op ln_opimpl_scatter_nocopy_cuda;
extern ln_op ln_opimpl_deconv2d_cuda;
extern ln_op ln_opimpl_submean_cuda;
/* end of declare cuda ops */

static ln_op *ops_cuda[] = {
    &ln_opimpl_create_cuda,
    &ln_opimpl_conv2d_cuda,
    &ln_opimpl_maxpool2d_cuda,
    &ln_opimpl_maxreduce_cuda,
    &ln_opimpl_relu_cuda,
    &ln_opimpl_reshape_cuda,
    &ln_opimpl_slice_cuda,
    &ln_opimpl_transpose_cuda,
    &ln_opimpl_zeros_cuda,
    &ln_opimpl_elew_cuda,
    &ln_opimpl_softmax_cuda,
    &ln_opimpl_concat_cuda,
    &ln_opimpl_batchnorm_cuda,
    &ln_opimpl_upsample_cuda,
    &ln_opimpl_maxreduce_arg_cuda,
    &ln_opimpl_print_cuda,
    &ln_opimpl_sigmoid_cuda,
    &ln_opimpl_sort1d_cuda,
    &ln_opimpl_sort1d_by_key_cuda,
    &ln_opimpl_arange_cuda,
    &ln_opimpl_transform_bboxSQD_cuda,
    &ln_opimpl_rearange_cuda,
    &ln_opimpl_pick1d_cuda,
    &ln_opimpl_fprint_cuda,
    &ln_opimpl_lrelu_cuda,
    &ln_opimpl_detect_yolov3_cuda,
    &ln_opimpl_avgpool2d_cuda,
    &ln_opimpl_resize_cuda,
    &ln_opimpl_gather_cuda,
    &ln_opimpl_gather_nocopy_cuda,
    &ln_opimpl_scatter_cuda,
    &ln_opimpl_scatter_nocopy_cuda,
    &ln_opimpl_deconv2d_cuda,
    &ln_opimpl_submean_cuda,
/* end of init cuda ops */
    NULL
};

static inline int can_replace(const char *optype)
{
    if (ln_streq(optype, "create") ||
        ln_streq(optype, "conv2d") ||
        ln_streq(optype, "maxpool2d") ||
        ln_streq(optype, "maxreduce") ||
        ln_streq(optype, "maxreduce_arg") ||
        ln_streq(optype, "relu") ||
        ln_streq(optype, "sigmoid") ||
        ln_streq(optype, "reshape") ||
        ln_streq(optype, "slice") ||
        ln_streq(optype, "transpose") ||
        ln_streq(optype, "zeros") ||
        ln_streq(optype, "elew") ||
        ln_streq(optype, "softmax") ||
        ln_streq(optype, "concat") ||
        ln_streq(optype, "upsample") ||
        ln_streq(optype, "sort1d") ||
        ln_streq(optype, "sort1d_by_key") ||
        ln_streq(optype, "arange") ||
        ln_streq(optype, "rearange") ||
        ln_streq(optype, "transform_bboxSQD") ||
        ln_streq(optype, "pick1d") ||
        ln_streq(optype, "lrelu") ||
        ln_streq(optype, "detect_yolov3") ||
        ln_streq(optype, "print"))
        return 1;
    return 0;

    /* for (int i = 0; ops_cuda[i]; i++) { */
    /*     if (ln_streq(ops_cuda[i]->op_arg->optype, optype)) */
    /*         return 1; */
    /* } */
    /* return 0; */
}

/* this is just for testing, should remove it */
static ln_list *cb_func_single_replace(const ln_context *ctx,
                                       const ln_list *ops, size_t size,
                                       int *match)
{
    ln_op *op, *new_op, *op_proto;
    ln_op_arg *op_arg;
    ln_list *new_ops;
    char *optype_cuda;
    int *replace_flag;
    size_t i;

    *match = 0;
    replace_flag = ln_alloc(sizeof(int) * size);
    i = 0;
    LN_LIST_FOREACH(op, ops) {
        if (i >= size)
            break;
        if (can_replace(op->op_arg->optype)) {
            replace_flag[i++] = 1;
            *match = 1;
            continue;
        }
        replace_flag[i++] = 0;
    }
    if (!match) {
        ln_free(replace_flag);
        return NULL;
    }

    new_ops = NULL;
    i = 0;
    LN_LIST_FOREACH(op, ops) {
        if (i >= size)
            break;
        op_arg = op->op_arg;
        if (!replace_flag[i++]) {
            new_op = ln_op_copy(op);
        } else {
            optype_cuda = ln_alloc(sizeof(char)*(strlen(op_arg->optype)+10));
            strcpy(optype_cuda, op_arg->optype);
            strcat(optype_cuda, "_cuda");
            op_proto = ln_op_array_find_by_optype(ops_cuda, optype_cuda);
            assert(op_proto && "optype_cuda not found");
            new_op = ln_op_create_from_proto(op_proto, op_arg->name,
                                             ln_tensor_list_copy(op_arg->tensors_in),
                                             ln_tensor_list_copy(op_arg->tensors_out),
                                             ln_param_list_copy(op_arg->params),
                                             op_arg->tensor_table);
            ln_free(optype_cuda);
        }
        new_ops = ln_list_append(new_ops, new_op);
    }

    ln_free(replace_flag);
    return new_ops;
}

extern ln_list *ln_expander_expander_cuda(const ln_context *ctx, const ln_op *op, int *match);
/* end of declare cuda expanders */

ln_expander_func ep_funcs_cuda[] = {
    ln_expander_expander_cuda,
/* end of cuda expanders */
    NULL
};

/* end of declare cuda combiners */

ln_combiner_func cb_funcs_cuda[] = {
    cb_func_single_replace,
/* end of cuda combiners */
    NULL
};

/* end of declare cuda subgraphers */

ln_subgraph_func sg_funcs_cuda[] = {
/* end of cuda subgraphers */
    NULL
};

/* end of declare cuda schedulers */

ln_schedule_func sd_funcs_cuda[] = {
/* end of cuda schedulers */
    NULL
};

extern void ln_expander_init_expander_cuda(void **priv_p);
/* end of declare cuda init funcs */

static void init_cuda(void **priv_p)
{
    ln_expander_init_expander_cuda(priv_p);
/* end of exec cuda init funcs */
}

extern void ln_expander_cleanup_expander_cuda(void **priv_p);
/* end of declare cuda cleanup funcs */

static void cleanup_cuda(void **priv_p)
{
    ln_expander_cleanup_expander_cuda(priv_p);
/* end of exec cuda cleanup funcs */
}

ln_arch ln_archimpl_cuda = {
    .init_func = init_cuda,
    .cleanup_func = cleanup_cuda,
    .reg_ops = ops_cuda,
    .ep_funcs = ep_funcs_cuda,
    .cb_funcs = cb_funcs_cuda,
    .sg_funcs = sg_funcs_cuda,
    .sd_funcs = sd_funcs_cuda,
    .arch_name = "cuda",
};

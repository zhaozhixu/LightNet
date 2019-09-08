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

/* NOTE: this file is automatically generated by protos/ep/ep_cpu.json
   using tools/addexpander.pl */

#include <assert.h>

#include "ln_arch.h"

 

static ln_list *ep_create(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "create_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_conv2d(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "conv2d_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_maxpool2d(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "maxpool2d_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_maxreduce(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "maxreduce_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_relu(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "relu_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_reshape(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "reshape_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_slice(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "slice_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_transpose(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "transpose_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_zeros(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "zeros_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_elew(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "elew_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_softmax(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "softmax_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_concat(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "concat_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_batchnorm(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "batchnorm_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_upsample(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "upsample_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_maxreduce_arg(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "maxreduce_arg_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_print(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "print_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_sigmoid(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "sigmoid_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_sort1d(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "sort1d_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_sort1d_by_key(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "sort1d_by_key_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_arange(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "arange_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_transform_bboxSQD(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "transform_bboxSQD_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_rearange(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "rearange_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_pick1d(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "pick1d_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_fprint(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "fprint_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_lrelu(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "lrelu_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_detect_yolov3(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "detect_yolov3_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_avgpool2d(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "avgpool2d_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_submean(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "submean_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_resize(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "resize_cpu");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_hash_init_entry init_ep_funcs[] = {
    {"create", ep_create},
    {"conv2d", ep_conv2d},
    {"maxpool2d", ep_maxpool2d},
    {"maxreduce", ep_maxreduce},
    {"relu", ep_relu},
    {"reshape", ep_reshape},
    {"slice", ep_slice},
    {"transpose", ep_transpose},
    {"zeros", ep_zeros},
    {"elew", ep_elew},
    {"softmax", ep_softmax},
    {"concat", ep_concat},
    {"batchnorm", ep_batchnorm},
    {"upsample", ep_upsample},
    {"maxreduce_arg", ep_maxreduce_arg},
    {"print", ep_print},
    {"sigmoid", ep_sigmoid},
    {"sort1d", ep_sort1d},
    {"sort1d_by_key", ep_sort1d_by_key},
    {"arange", ep_arange},
    {"transform_bboxSQD", ep_transform_bboxSQD},
    {"rearange", ep_rearange},
    {"pick1d", ep_pick1d},
    {"fprint", ep_fprint},
    {"lrelu", ep_lrelu},
    {"detect_yolov3", ep_detect_yolov3},
    {"avgpool2d", ep_avgpool2d},
    {"submean", ep_submean},
    {"resize", ep_resize},
    LN_HASH_INIT_ENTRY_NULL
};
static ln_hash *ep_funcs_hash = NULL;

void ln_expander_init_expander_cpu(void **priv_p)
{
    ep_funcs_hash = ln_hash_create(ln_str_hash, ln_str_cmp, NULL, NULL);
    ln_hash_init(ep_funcs_hash, init_ep_funcs);
}

void ln_expander_cleanup_expander_cpu(void **priv_p)
{
    ln_hash_free(ep_funcs_hash);
}

ln_list *ln_expander_expander_cpu(const ln_context *ctx, const ln_op *self, int *match)
{
    ln_expander_func  ep_func;
    ln_list          *new_ops;
    void             *value;

    if (!ln_hash_find_extended(ep_funcs_hash, self->op_arg->optype, NULL, &value))
        ln_msg_inter_error("unsupported optype '%s' for expander_cpu expander",
                           self->op_arg->optype);

    ep_func = value;
    new_ops = ep_func(ctx, self, match);

    return new_ops;
}

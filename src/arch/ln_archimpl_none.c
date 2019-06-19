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

extern ln_op ln_opimpl_slice;
extern ln_op ln_opimpl_reshape;
extern ln_op ln_opimpl_maxreduce;
extern ln_op ln_opimpl_elew;
extern ln_op ln_opimpl_transpose;
extern ln_op ln_opimpl_zeros;
extern ln_op ln_opimpl_create;
extern ln_op ln_opimpl_conv2d;
extern ln_op ln_opimpl_relu;
extern ln_op ln_opimpl_maxpool2d;
extern ln_op ln_opimpl_softmax;
extern ln_op ln_opimpl_concat;
extern ln_op ln_opimpl_batchnorm;
extern ln_op ln_opimpl_upsample;
extern ln_op ln_opimpl_maxreduce_arg;
extern ln_op ln_opimpl_print;
extern ln_op ln_opimpl_sigmoid;
extern ln_op ln_opimpl_sort1d;
extern ln_op ln_opimpl_sort1d_by_key;
extern ln_op ln_opimpl_arange;
extern ln_op ln_opimpl_transform_bboxSQD;
extern ln_op ln_opimpl_rearange;
extern ln_op ln_opimpl_pick1d;
extern ln_op ln_opimpl_fprint;
extern ln_op ln_opimpl_lrelu;
extern ln_op ln_opimpl_detect_yolov3;
extern ln_op ln_opimpl_avgpool2d;
extern ln_op ln_opimpl_resize;
extern ln_op ln_opimpl_gather;
extern ln_op ln_opimpl_scatter;
extern ln_op ln_opimpl_svmr;
extern ln_op ln_opimpl_ldmr;
extern ln_op ln_opimpl_deconv2d;
/* end of declare none ops */

/* TODO: use a hash */
static ln_op *ops_none[] = {
    &ln_opimpl_slice,
    &ln_opimpl_reshape,
    &ln_opimpl_maxreduce,
    &ln_opimpl_elew,
    &ln_opimpl_transpose,
    &ln_opimpl_zeros,
    &ln_opimpl_create,
    &ln_opimpl_conv2d,
    &ln_opimpl_relu,
    &ln_opimpl_maxpool2d,
    &ln_opimpl_softmax,
    &ln_opimpl_concat,
    &ln_opimpl_batchnorm,
    &ln_opimpl_upsample,
    &ln_opimpl_maxreduce_arg,
    &ln_opimpl_print,
    &ln_opimpl_sigmoid,
    &ln_opimpl_sort1d,
    &ln_opimpl_sort1d_by_key,
    &ln_opimpl_arange,
    &ln_opimpl_transform_bboxSQD,
    &ln_opimpl_rearange,
    &ln_opimpl_pick1d,
    &ln_opimpl_fprint,
    &ln_opimpl_lrelu,
    &ln_opimpl_detect_yolov3,
    &ln_opimpl_avgpool2d,
    &ln_opimpl_resize,
    &ln_opimpl_gather,
    &ln_opimpl_scatter,
    &ln_opimpl_svmr,
    &ln_opimpl_ldmr,
    &ln_opimpl_deconv2d,
/* end of init none ops */
    NULL
};

ln_expander_func ep_funcs_none[] = {
    NULL
};

ln_combiner_func cb_funcs_none[] = {
    NULL
};

/* end of declare none subgraphers */

ln_subgraph_func sg_funcs_none[] = {
/* end of none subgraphers */
    NULL
};

/* end of declare none schedulers */

ln_schedule_func sd_funcs_none[] = {
/* end of none schedulers */
    NULL
};

ln_arch ln_archimpl_none = {
    .init_func = NULL,
    .cleanup_func = NULL,
    .reg_ops = ops_none,
    .ep_funcs = ep_funcs_none,
    .cb_funcs = cb_funcs_none,
    .sg_funcs = sg_funcs_none,
    .sd_funcs = sd_funcs_none,
    .arch_name = "none",
};

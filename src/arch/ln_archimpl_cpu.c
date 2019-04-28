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

extern ln_op ln_opimpl_arange_cpu;
extern ln_op ln_opimpl_avgpool2d_cpu;
extern ln_op ln_opimpl_batchnorm_cpu;
extern ln_op ln_opimpl_bn2scale_wts_cpu;
extern ln_op ln_opimpl_concat_cpu;
extern ln_op ln_opimpl_conv2d_cpu;
extern ln_op ln_opimpl_create_cpu;
extern ln_op ln_opimpl_detect_yolov3_cpu;
extern ln_op ln_opimpl_elew_cpu;
extern ln_op ln_opimpl_fprint_cpu;
extern ln_op ln_opimpl_lrelu_cpu;
extern ln_op ln_opimpl_maxpool2d_cpu;
extern ln_op ln_opimpl_maxreduce_arg_cpu;
extern ln_op ln_opimpl_maxreduce_cpu;
extern ln_op ln_opimpl_pick1d_cpu;
extern ln_op ln_opimpl_print_cpu;
extern ln_op ln_opimpl_rearange_cpu;
extern ln_op ln_opimpl_relu_cpu;
extern ln_op ln_opimpl_reshape_cpu;
extern ln_op ln_opimpl_sigmoid_cpu;
extern ln_op ln_opimpl_slice_cpu;
extern ln_op ln_opimpl_softmax_cpu;
extern ln_op ln_opimpl_sort1d_by_key_cpu;
extern ln_op ln_opimpl_sort1d_cpu;
extern ln_op ln_opimpl_transform_bboxSQD_cpu;
extern ln_op ln_opimpl_transpose_cpu;
extern ln_op ln_opimpl_upsample_cpu;
extern ln_op ln_opimpl_zeros_cpu;
extern ln_op ln_opimpl_resize_cpu;
/* end of declare cpu ops */

static ln_op *ops_cpu[] = {
    &ln_opimpl_arange_cpu,
    &ln_opimpl_avgpool2d_cpu,
    &ln_opimpl_batchnorm_cpu,
    &ln_opimpl_bn2scale_wts_cpu,
    &ln_opimpl_concat_cpu,
    &ln_opimpl_conv2d_cpu,
    &ln_opimpl_create_cpu,
    &ln_opimpl_detect_yolov3_cpu,
    &ln_opimpl_elew_cpu,
    &ln_opimpl_fprint_cpu,
    &ln_opimpl_lrelu_cpu,
    &ln_opimpl_maxpool2d_cpu,
    &ln_opimpl_maxreduce_arg_cpu,
    &ln_opimpl_maxreduce_cpu,
    &ln_opimpl_pick1d_cpu,
    &ln_opimpl_print_cpu,
    &ln_opimpl_rearange_cpu,
    &ln_opimpl_relu_cpu,
    &ln_opimpl_reshape_cpu,
    &ln_opimpl_sigmoid_cpu,
    &ln_opimpl_slice_cpu,
    &ln_opimpl_softmax_cpu,
    &ln_opimpl_sort1d_by_key_cpu,
    &ln_opimpl_sort1d_cpu,
    &ln_opimpl_transform_bboxSQD_cpu,
    &ln_opimpl_transpose_cpu,
    &ln_opimpl_upsample_cpu,
    &ln_opimpl_zeros_cpu,
    &ln_opimpl_resize_cpu,
/* end of init cpu ops */
    NULL
};

extern ln_list *ln_expander_expander_cpu(const ln_op *op, const ln_dfg *dfg, int *match);
/* end of declare cpu expanders */

ln_expander_func ep_funcs_cpu[] = {
    ln_expander_expander_cpu,
/* end of cpu expanders */
    NULL
};

/* end of declare cpu combiners */

ln_combiner_func cb_funcs_cpu[] = {
/* end of cpu combiners */
    NULL
};

extern void ln_expander_init_expander_cpu(void **priv_p);
/* end of declare cpu init funcs */

static void init_cpu(void **priv_p)
{
    ln_expander_init_expander_cpu(priv_p);
/* end of exec cpu init funcs */
}

extern void ln_expander_cleanup_expander_cpu(void **priv_p);
/* end of declare cpu cleanup funcs */

static void cleanup_cpu(void **priv_p)
{
    ln_expander_cleanup_expander_cpu(priv_p);
/* end of exec cpu cleanup funcs */
}

ln_arch ln_archimpl_cpu = {
    .init_func = init_cpu,
    .cleanup_func = cleanup_cpu,
    .reg_ops = ops_cpu,
    .ep_funcs = ep_funcs_cpu,
    .cb_funcs = cb_funcs_cpu,
    .arch_name = "cpu",
};

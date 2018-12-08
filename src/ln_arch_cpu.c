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

#include "ln_arch.h"

extern ln_op ln_opimpl_reshape_cpu;
extern ln_op ln_opimpl_create_cpu;
extern ln_op ln_opimpl_bn2scale_wts_cpu;
extern ln_op ln_opimpl_upsample_cpu;
extern ln_op ln_opimpl_batchnorm_cpu;
extern ln_op ln_opimpl_concat_cpu;
extern ln_op ln_opimpl_conv2d_cpu;
extern ln_op ln_opimpl_elew_cpu;
extern ln_op ln_opimpl_maxpool2d_cpu;
extern ln_op ln_opimpl_maxreduce_arg_cpu;
extern ln_op ln_opimpl_maxreduce_cpu;
extern ln_op ln_opimpl_relu_cpu;
extern ln_op ln_opimpl_slice_cpu;
extern ln_op ln_opimpl_softmax_cpu;
extern ln_op ln_opimpl_transpose_cpu;
extern ln_op ln_opimpl_zeros_cpu;
extern ln_op ln_opimpl_print_cpu;
/* end of declare cpu ops */

/* TODO: use a hash */
static ln_op *ops_cpu[] = {
    &ln_opimpl_reshape_cpu,
    &ln_opimpl_create_cpu,
    &ln_opimpl_bn2scale_wts_cpu,
    &ln_opimpl_upsample_cpu,
    &ln_opimpl_batchnorm_cpu,
    &ln_opimpl_concat_cpu,
    &ln_opimpl_conv2d_cpu,
    &ln_opimpl_elew_cpu,
    &ln_opimpl_maxpool2d_cpu,
    &ln_opimpl_maxreduce_arg_cpu,
    &ln_opimpl_maxreduce_cpu,
    &ln_opimpl_relu_cpu,
    &ln_opimpl_slice_cpu,
    &ln_opimpl_softmax_cpu,
    &ln_opimpl_transpose_cpu,
    &ln_opimpl_zeros_cpu,
    &ln_opimpl_print_cpu,
/* end of init cpu ops */
    NULL
};

ln_expander_func ep_funcs_cpu[] = {
    NULL
};

ln_combiner_func cb_funcs_cpu[] = {
    NULL
};

ln_arch ln_arch_cpu = {
    .reg_ops = ops_cpu,
    .init_func = NULL,
    .cleanup_func = NULL,
    .ep_funcs = ep_funcs_cpu,
    .cb_funcs = cb_funcs_cpu,
    .arch_name = "cpu",
};

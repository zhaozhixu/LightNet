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
/* end of declare cpu ops */

static ln_op *ops_cpu[] = {
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
/* end of init cpu ops */
     NULL
};

ln_peephole_func ph_funcs_cpu[] = {
     NULL
};

ln_arch ln_arch_cpu = {
     .ops = ops_cpu,
     .ph_funcs = ph_funcs_cpu,
     .arch_name = "cpu",
};

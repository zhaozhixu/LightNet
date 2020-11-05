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

#include "ln_arch.h"
#include "ln_cudnn.h"

extern ln_op ln_opimpl_relu_cudnn;
/* end of declare cudnn ops */

static ln_op *ops_cudnn[] = {
    &ln_opimpl_relu_cudnn,
/* end of init cudnn ops */
    NULL
};

static void init_cudnn(void **priv_p)
{
    *priv_p = ln_cudnn_context_create();
}

static void cleanup_cudnn(void **priv_p)
{
    ln_cudnn_context_free(*priv_p);
}

static void optimize_cudnn (ln_context *ctx, const char *datafile)
{
    /* nothing here */
}

ln_arch ln_archimpl_cudnn = {
    .arch_name = "cudnn",
    .priv = NULL,
    .reg_ops = ops_cudnn,
    .init_func = init_cudnn,
    .cleanup_func = cleanup_cudnn,
    .optimize_func = optimize_cudnn,
};

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

#include "ln_arch.h"

/* end of declare tensorrt ops */

static ln_op *ops_tensorrt[] = {
/* end of init tensorrt ops */
    NULL
};

/* end of declare tensorrt expanders */

ln_expander_func ep_funcs_tensorrt[] = {
/* end of tensorrt expanders */
    NULL
};

/* end of declare tensorrt combiners */

ln_combiner_func cb_funcs_tensorrt[] = {
/* end of tensorrt combiners */
    NULL
};

/* end of declare tensorrt init funcs */

static void init_tensorrt(void **context_p)
{
/* end of exec tensorrt init funcs */
}

/* end of declare tensorrt cleanup funcs */

static void cleanup_tensorrt(void **context_p)
{
/* end of exec tensorrt cleanup funcs */
}

ln_arch ln_archimpl_tensorrt = {
    .init_func = init_tensorrt,
    .cleanup_func = cleanup_tensorrt,
    .reg_ops = ops_tensorrt,
    .ep_funcs = ep_funcs_tensorrt,
    .cb_funcs = cb_funcs_tensorrt,
    .arch_name = "tensorrt",
};

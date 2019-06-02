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

/* end of declare dpu ops */

static ln_op *ops_dpu[] = {
/* end of init dpu ops */
    NULL
};

/* end of declare dpu expanders */

ln_expander_func ep_funcs_dpu[] = {
/* end of dpu expanders */
    NULL
};

/* end of declare dpu combiners */

ln_combiner_func cb_funcs_dpu[] = {
/* end of dpu combiners */
    NULL
};

/* end of declare dpu subgraphers */

ln_subgraph_func sg_funcs_dpu[] = {
/* end of dpu subgraphers */
    NULL
};

/* end of declare dpu schedulers */

ln_schedule_func sd_funcs_dpu[] = {
/* end of dpu schedulers */
    NULL
};

/* end of declare dpu init funcs */

static void init_dpu(void **priv_p)
{
/* end of exec dpu init funcs */
}

/* end of declare dpu cleanup funcs */

static void cleanup_dpu(void **priv_p)
{
/* end of exec dpu cleanup funcs */
}

ln_arch ln_archimpl_dpu = {
    .init_func = init_dpu,
    .cleanup_func = cleanup_dpu,
    .reg_ops = ops_dpu,
    .ep_funcs = ep_funcs_dpu,
    .cb_funcs = cb_funcs_dpu,
    .sg_funcs = sg_funcs_dpu,
    .sd_funcs = sd_funcs_dpu,
    .arch_name = "dpu",
};

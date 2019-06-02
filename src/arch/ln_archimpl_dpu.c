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

ln_list *ln_subgrapher_dpu(const ln_list *ops, const ln_dfg *dfg,
                           ln_list **old_ops)
{
    ln_list *new_ops = NULL;
    ln_tensor_list_entry *tle;
    ln_op *op;

    /* find op in pattern svmr->concat->slice->ldmr->conv2d in `ops` using `dfg`
       and put them in `old_ops`, such as: */
    LN_LIST_FOREACH(op, ops) {
        if (/* found an op! */) {
            *old_ops = ln_list_prepend(*old_ops, op);
        }
    }

    /* use `ln_op *ln_op_copy(const ln_op *op)` to copy old conv2ds,
       free their old input tensor names and assign new names: */
    LN_LIST_FOREACH(tle, op->op_arg->tensors_in) {
        ln_free(tle->name);
        /* new tensor names are previous conv2ds' output tensor names */
        tle->name = ln_strdup("new tensor name");
    }

    return new_ops;
}

ln_list *ln_scheduler_dpu(const ln_dfg *dfg)
{

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

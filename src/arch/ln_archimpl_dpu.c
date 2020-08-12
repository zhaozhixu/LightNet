/*
 * Copyright (c) 2018-2020 Zhao Zhixu
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

extern ln_op ln_opimpl_gather_nocopy_dpu;
extern ln_op ln_opimpl_scatter_nocopy_dpu;
extern ln_op ln_opimpl_svmr_dpu;
extern ln_op ln_opimpl_ldmr_dpu;
extern ln_op ln_opimpl_conv1x1_dpu;
extern ln_op ln_opimpl_conv3x3_dpu;
/* end of declare dpu ops */

static ln_op *ops_dpu[] = {
    &ln_opimpl_gather_nocopy_dpu,
    &ln_opimpl_scatter_nocopy_dpu,
    &ln_opimpl_svmr_dpu,
    &ln_opimpl_ldmr_dpu,
    &ln_opimpl_conv1x1_dpu,
    &ln_opimpl_conv3x3_dpu,
/* end of init dpu ops */
    NULL
};

ln_list *ln_expander_dpu(const ln_context *ctx, const ln_op *self, int *match);
/* end of declare dpu expanders */

ln_list *ln_subgrapher_dpu(const ln_list *ops, const ln_dfg *dfg,
                           ln_list **old_ops);
/* end of declare dpu subgraphers */

ln_list *ln_scheduler_dpu(const ln_dfg *dfg);
/* end of declare dpu schedulers */

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

static ln_op *create_new_op(const char *optype, const ln_context *ctx)
{
    ln_op *op_proto;

    op_proto = ln_hash_find(LN_ARCH.op_proto_table, optype);
    if (!op_proto)
        ln_msg_inter_error("can't find op proto '%s'", optype);

    return ln_op_create_with_names(op_proto, ctx->ops, ctx->tensor_table);
}

static ln_list *ep_conv2d(const ln_op *self, const ln_context *ctx)
{
    int last_index;
    ln_op *transpose;
    ln_op *reshape;
    ln_op *scatter;
    ln_op *gather;
    ln_op *ldmr;
    ln_op *svmr;
    ln_op *conv_dpu;
    ln_list *new_ops = NULL;

    transpose = create_new_op("transpose_cpu", ctx);
    reshape = create_new_op("reshape_cpu", ctx);
    scatter = create_new_op("scatter", ctx);
}

ln_list *ln_expander_dpu(const ln_context *ctx, const ln_op *self, int *match)
{
    ln_list *new_ops = NULL;

    *match = 0;
    if (ln_streq(self->op_arg->optype, "conv2d")) {
        *match = 1;
        new_ops = ep_conv2d(self, ctx);
    }

    return new_ops;
}

/* static int check_prevs(const ln_dfg *dfg, const ln_op *op, const char *optype) */
/* { */
/*     ln_tensor_list_entry *tle; */
/*     ln_op *prev_op; */

/*     LN_LIST_FOREACH(tle, op->op_arg->tensors_in) { */
/*         prev_op = ln_dfg_prev(dfg, op, tle->name); */
/*         if (!ln_streq(prev_op->op_arg->optype, optype)) */
/*             return 0; */
/*     } */
/*     return 1; */
/* } */

/* ln_list *ln_subgrapher_dpu(const ln_list *ops, const ln_dfg *dfg, */
/*                            ln_list **old_ops) */
/* { */
/*     ln_list *new_ops = NULL; */
/*     ln_tensor_list_entry *tle; */
/*     ln_op *op; */

/*     /\* find op in pattern svmr->concat->slice->ldmr->conv2d in `ops` using `dfg` */
/*        and put them in `old_ops`, such as: *\/ */
/*     LN_LIST_FOREACH(op, ops) { */
/*         if (!ln_streq(op->op_arg->optype, "concat")) */
/*             continue; */
/*         LN_LIST_FOREACH(tle, op->op_arg->tensors_in) { */
/*             prev_op = ln_dfg_prev(dfg, op, tle->name); */
/*             if (!ln_streq(op->)) */
/*         } */
/*         if (1/\* found an op! *\/) { */
/*             *old_ops = ln_list_prepend(*old_ops, op); */
/*         } */
/*     } */

/*     /\* use `ln_op *ln_op_copy(const ln_op *op)` to copy old conv2ds, */
/*        free their old input tensor names and assign new names: *\/ */
/*     LN_LIST_FOREACH(tle, op->op_arg->tensors_in) { */
/*         ln_free(tle->name); */
/*         /\* new tensor names are previous conv2ds' output tensor names *\/ */
/*         tle->name = ln_strdup("new tensor name"); */
/*     } */

/*     return new_ops; */
/* } */

ln_list *ln_scheduler_dpu(const ln_dfg *dfg)
{
    ln_list *ops = NULL;

    if (ln_graph_dft_after_prev(dfg->graph, &ops) < 0)
        ln_msg_error("graph has a circle when doing Depth-First Traversal");

    return ops;
}

static void optimize_dpu (ln_context *ctx, const char *datafile)
{
    ln_pass_preprocess(ctx);
    ln_pass_expander(ctx, ln_expander_dpu);
    ln_pass_preprocess(ctx);
    /* ln_pass_combiner(ctx, 2, cb_func_dpu); */
    ln_pass_subgraph(ctx, ln_subgrapher_dpu);
    /* ln_pass_schedule(ctx, ln_scheduler_dpu); */

    /* make ops consistent */
    ln_op_list_do_post_run(ctx->ops);
    assert(ln_hash_size(ctx->tensor_table) == 0);
    ln_op_list_do_pre_run(ctx->ops);

    ln_pass_mem_plan(ctx);
    /* ln_context_print(ctx, "out_debug.json"); */
}

ln_arch ln_archimpl_dpu = {
    .arch_name = "dpu",
    .priv = NULL,
    .reg_ops = ops_dpu,
    .init_func = init_dpu,
    .cleanup_func = cleanup_dpu,
    .optimize_func = optimize_dpu,
};

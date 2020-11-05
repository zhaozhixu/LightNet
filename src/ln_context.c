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

#include <stdarg.h>
#include "ln_context.h"
#include "ln_json.h"
#include "ln_pass.h"

LN_EXPORT ln_context *ln_context_create(void)
{
    ln_context *ctx;

    ctx = ln_alloc(sizeof(ln_context));
    ctx->tensor_table = ln_tensor_table_create();
    ctx->op_table = ln_op_table_create();
    ctx->dfg = ln_dfg_create();
    ctx->ops = NULL;
    memset(ctx->mem_starts, 0, sizeof(ctx->mem_starts));
    memset(ctx->mem_sizes, 0, sizeof(ctx->mem_sizes));

    return ctx;
}

LN_EXPORT void ln_context_free(ln_context *ctx)
{
    ln_tensor_table_free(ctx->tensor_table);
    ln_op_table_free(ctx->op_table);
    ln_dfg_free(ctx->dfg);
    ln_op_list_free(ctx->ops);
    ln_free(ctx);
}


LN_EXPORT void ln_context_init(ln_context *ctx, const char *source)
{
    ln_json_parse_file(source, ctx);
    ln_context_init_ops(ctx);
}

LN_EXPORT void ln_context_compile(ln_context *ctx, const char *target, const char *datafile)
{
    ln_arch *arch;

    arch = ln_hash_find(LN_ARCH.arch_table, target);
    if (!arch->optimize_func)
        return;
    arch->optimize_func(ctx, datafile);
}

LN_EXPORT void ln_context_print(const ln_context *ctx, const char *outfile)
{
    if (ln_streq(outfile, "-"))
        ln_json_fprint(stdout, ctx);
    else
        ln_json_print_file(outfile, ctx);
}

LN_EXPORT void ln_context_load(ln_context *ctx, const char *datafile)
{
    ln_context_alloc_mem(ctx);
    if (datafile)
        ln_tensor_table_load_trt_weight_file(ctx->tensor_table, datafile);
    ln_op_list_do_static_run(ctx->ops);
}

LN_EXPORT void ln_context_set_data(ln_context *ctx, const char *tname, const void *data)
{
    ln_tensor_table_set_data(ctx->tensor_table, tname, data);
}

LN_EXPORT void *ln_context_get_data(ln_context *ctx, const char *tname, void *data)
{
    return ln_tensor_table_get_data(ctx->tensor_table, tname, data);
}

LN_EXPORT size_t ln_context_data_size(ln_context *ctx, const char *tname)
{
    return ln_tensor_table_data_size(ctx->tensor_table, tname);
}

LN_EXPORT void ln_context_set_param(ln_context *ctx, const char *opname,
                          const char *pname, ...)
{
    va_list ap;

    va_start(ap, pname);
    ln_op_table_vset_param(ctx->op_table, opname, pname, ap);
    va_end(ap);
}

LN_EXPORT void ln_context_run(const ln_context *ctx)
{
    /* LN_TIMEIT_START; */
    ln_op_list_do_run(ctx->ops);
    /* LN_TIMEIT_END("time of ln_context_run(): "); */
}

LN_EXPORT void ln_context_unload(ln_context *ctx)
{
    ln_context_dealloc_mem(ctx);
}

LN_EXPORT void ln_context_cleanup(ln_context *ctx)
{
    ln_context_cleanup_ops(ctx);
}

static void init_op(ln_context *ctx, ln_op *op)
{
    int ret;

    ln_msg_debug("init_op: %s (%s)", op->op_arg->name, op->op_arg->optype);
    ret = ln_op_table_insert(ctx->op_table, op);
    assert(ret);
    op->pre_run(op->op_arg);
    ln_dfg_add(ctx->dfg, op);
}

static void cleanup_op(ln_context *ctx, ln_op *op)
{
    int ret;

    ln_msg_debug("cleanup_op: %s (%s)", op->op_arg->name, op->op_arg->optype);
    ln_dfg_remove(ctx->dfg, op);
    op->post_run(op->op_arg);
    ret = ln_op_table_remove(ctx->op_table, op->op_arg->name);
    assert(ret);
}

void ln_context_init_ops(ln_context *ctx)
{
    ln_op *op;

    LN_LIST_FOREACH(op, ctx->ops) {
        init_op(ctx, op);
    }
    ln_context_check(ctx);
}

void ln_context_cleanup_ops(ln_context *ctx)
{
    ln_op *op;

    LN_LIST_FOREACH(op, ctx->ops) {
        cleanup_op(ctx, op);
    }
    ln_context_check(ctx);
}

void ln_context_replace_ops(ln_context *ctx, ln_list **position,
                            size_t len, ln_list *new_ops)
{
    ln_op *op;
    size_t i;
    ln_list **lp;
    ln_list *tmp;

    for (i = 0, lp = position; i < len && *lp; i++) {
        op = (*lp)->data;
        cleanup_op(ctx, op);
        tmp = *lp;
        *lp = tmp->next;
        ln_free(tmp);
    }

    tmp = *position;
    *position = new_ops;
    for (lp = position; *lp; lp = &(*lp)->next) {
        op = (*lp)->data;
        init_op(ctx, op);
    }
    *lp = tmp;
}

void ln_context_remove_op(ln_context *ctx, ln_list **position)
{
    ln_context_replace_ops(ctx, position, 1, NULL);
}

void ln_context_add_op(ln_context *ctx, ln_list **position, ln_op *new_op)
{
    ln_list *list = NULL;
    list = ln_list_append(list, new_op);
    ln_context_replace_ops(ctx, position, 0, list);
}

void ln_context_subgraph(ln_context *ctx, ln_list *old_ops, ln_list *new_ops)
{
    ln_op *op;
    ln_list *layers = NULL;
    ln_list *layer;
    ln_list *ops = NULL;

    LN_LIST_FOREACH(op, old_ops) {
        cleanup_op(ctx, op);
    }

    LN_LIST_FOREACH(op, new_ops) {
        init_op(ctx, op);
    }

    if (ln_graph_topsort(ctx->dfg->graph, &layers) < 0)
        ln_msg_error("DFG has a circle during graph substitution");

    LN_LIST_FOREACH(layer, layers) {
        LN_LIST_FOREACH(op, layer) {
            ops = ln_list_append(ops, op);
        }
    }
    ln_graph_free_topsortlist(layers);

    ln_list_free(ctx->ops);
    ctx->ops = ops;
}

int ln_context_check(const ln_context *ctx)
{
    return ln_dfg_check(ctx->dfg);
    /* if (!ln_dfg_check(ctx->dfg)) */
    /*     ln_context_print(ctx, "-"); */
    /* return 0; */
}

void ln_context_alloc_mem(ln_context *ctx)
{
    ln_op *op;
    ln_tensor_list_entry *tle;
    ln_tensor_entry *te;
    size_t water_level;
    int i;

    for (i = LN_MEM_NONE+1; i < LN_MEM_TYPE_SIZE; i++) {
        if (ctx->mem_sizes[i] == 0)
            continue;
        ctx->mem_starts[i] = ln_mem_type_info(i).alloc_func(ctx->mem_sizes[i]);
        if (ln_mem_type_info(i).memset_func)
            ln_mem_type_info(i).memset_func(ctx->mem_starts[i], 0,
                                            ctx->mem_sizes[i]);
        ln_msg_debug("allocate memory %s: %lu bytes at address %p",
                     ln_mem_type_name(i), ctx->mem_sizes[i],
                     ctx->mem_starts[i]);
        if (ctx->mem_sizes[i])
            assert(ctx->mem_starts[i]);
    }
    LN_LIST_FOREACH(op, ctx->ops) {
        LN_LIST_FOREACH(tle, op->op_arg->tensors_out) {
            te = ln_tensor_table_find(op->op_arg->tensor_table, tle->name);
            assert(te);
            if (te->mtype == LN_MEM_NONE)
                ln_msg_error("tensor '%s' has memory type LN_MEM_NONE",
                             te->name);
            if (te->offset == 0)
                ln_msg_error("invalid data offset %p of tensor '%s'",
                             te->offset, te->name);
            water_level = te->offset + tl_tensor_size(te->tensor);
            if (water_level > ctx->mem_sizes[te->mtype])
                ln_msg_error("data of tensor '%s' exceeds planned %s memory size %lu by %lu bytes",
                             te->name, ln_mem_type_name(te->mtype),
                             ctx->mem_sizes[te->mtype],
                             water_level - ctx->mem_sizes[te->mtype]);
            te->tensor->data = te->offset + ctx->mem_starts[te->mtype];
        }
    }
}

void ln_context_dealloc_mem(ln_context *ctx)
{
    int i;

    for (i = LN_MEM_NONE+1; i < LN_MEM_TYPE_SIZE; i++) {
        ln_msg_debug("free memory %s: %lu bytes at address %p",
                     ln_mem_type_name(i), ctx->mem_sizes[i],
                     ctx->mem_starts[i]);
        if (ctx->mem_starts[i] == 0)
            continue;
        ln_mem_type_info(i).free_func(ctx->mem_starts[i]);
        ctx->mem_starts[i] = 0;
    }
}

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

#include "ln_context.h"
#include "ln_json.h"
#include "ln_pass.h"

ln_context *ln_context_create(void)
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

void ln_context_free(ln_context *ctx)
{
    ln_tensor_table_free(ctx->tensor_table);
    ln_op_table_free(ctx->op_table);
    ln_dfg_free(ctx->dfg);
    ln_op_list_free(ctx->ops);
}

static void init_op(ln_context *ctx, ln_op *op)
{
    int ret;

    /* ln_msg_debug("init_op: %s (%s)", op->op_arg->name, op->op_arg->optype); */
    ret = ln_op_table_insert(ctx->op_table, op);
    assert(ret);
    op->pre_run(op->op_arg);
    ln_dfg_add(ctx->dfg, op);
}

static void cleanup_op(ln_context *ctx, ln_op *op)
{
    int ret;

    /* ln_msg_debug("cleanup_op: %s (%s)", op->op_arg->name, op->op_arg->optype); */
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

void ln_context_replace_ops(ln_context *ctx, ln_list **start_p, size_t len,
                            ln_list *new_ops)
{
    ln_op *op;
    size_t i;
    ln_list **lp;
    ln_list *tmp;

    for (i = 0, lp = start_p; i < len && *lp; i++) {
        op = (*lp)->data;
        cleanup_op(ctx, op);
        tmp = *lp;
        *lp = tmp->next;
        ln_free(tmp);
    }

    tmp = *start_p;
    *start_p = new_ops;
    for (lp = start_p; *lp; lp = &(*lp)->next) {
        op = (*lp)->data;
        init_op(ctx, op);
    }
    *lp = tmp;

    ln_context_check(ctx);
}

int ln_context_check(const ln_context *ctx)
{
    return ln_dfg_check(ctx->dfg);
}

void ln_context_alloc_mem(ln_context *ctx)
{
    ln_op *op;
    ln_tensor_list_entry *tle;
    ln_tensor_entry *te;
    size_t water_level;
    int i;

    for (i = LN_MEM_NONE+1; i < LN_MEM_TYPE_SIZE; i++) {
        ctx->mem_starts[i] = ln_mtype_infos[i].alloc_func(ctx->mem_sizes[i]);
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
        ln_mtype_infos[i].free_func(ctx->mem_starts[i]);
        ctx->mem_starts[i] = 0;
    }
}

void ln_context_init(ln_context *ctx, const char *source)
{
    ln_json_parse_file(source, ctx);
    ln_context_init_ops(ctx);
}

void ln_context_compile(ln_context *ctx, const char *target)
{
    ln_arch *arch;

    arch = ln_hash_find(LN_ARCH.arch_table, target);
    ln_pass_expander(ctx, arch->ep_funcs);
    ln_pass_combiner(ctx, 3, arch->cb_funcs);

    /* make ops consistent */
    ln_op_list_do_post_run(ctx->ops);
    assert(ln_hash_size(ctx->tensor_table) == 0);
    ln_op_list_do_pre_run(ctx->ops);
    ln_pass_mem_plan(ctx);
}

void ln_context_print(const ln_context *ctx, const char *outfile)
{
    if (ln_streq(outfile, "-"))
        ln_json_fprint(stdout, ctx);
    else
        ln_json_print_file(outfile, ctx);
}

void ln_context_load(ln_context *ctx)
{
    ln_context_alloc_mem(ctx);
    ln_op_list_do_static_run(ctx->ops);
}

void ln_context_run(const ln_context *ctx)
{
    ln_op_list_do_run(ctx->ops);
}

void ln_context_unload(ln_context *ctx)
{
    ln_context_dealloc_mem(ctx);
}

void ln_context_cleanup(ln_context *ctx)
{
    ln_context_cleanup_ops(ctx);
}

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

ln_context *ln_context_create(void)
{
    ln_context *ctx;

    ctx = ln_alloc(sizeof(ln_context));
    ctx->tensor_table = ln_tensor_table_create();
    ctx->op_table = ln_op_table_create();
    ctx->dfg = ln_dfg_create();
    ctx->ops = NULL;

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
    ln_error *error = NULL;

    /* ln_error_debug("init_op: %s %s\n", op->op_arg->name, op->op_arg->optype); */
    op->pre_run(op->op_arg, &error);
    ln_error_handle(&error);
    ret = ln_op_table_insert(ctx->op_table, op);
    assert(ret);
    ln_dfg_add(ctx->dfg, op);
}

static void cleanup_op(ln_context *ctx, ln_op *op)
{
    int ret;
    ln_error *error = NULL;

    /* ln_error_debug("cleanup_op: %s %s\n", op->op_arg->name, op->op_arg->optype); */
    ln_dfg_remove(ctx->dfg, op);
    op->post_run(op->op_arg, &error);
    ln_error_handle(&error);
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

int ln_context_check(ln_context *ctx)
{
    return ln_dfg_check(ctx->dfg);
}

void ln_context_run(ln_context *ctx)
{
    ln_error *error = NULL;

    ln_op_list_do_static_run(ctx->ops, &error);
    ln_error_handle(&error);
    ln_op_list_do_run(ctx->ops, &error);
    ln_error_handle(&error);
}

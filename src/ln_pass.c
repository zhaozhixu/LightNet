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

#include <assert.h>
#include "ln_pass.h"

const int MAX_PEEPHOLE_PASSES = 3;

void ln_pass_expander(ln_context *ctx, const ln_expander_func *ep_funcs)
{
    ln_op *op;
    ln_list **lp;
    ln_list *ep_ops;
    ln_expander_func ep_func;
    int match;
    int i;

    ep_ops = NULL;
    for (lp = &ctx->ops; *lp; lp = &(*lp)->next) {
        op = (*lp)->data;
        for (i = 0; (ep_func = ep_funcs[i]); i++) {
            match = 0;
            ep_ops = ep_func(op, ctx->dfg, &match);
            if (!match)
                continue;
            ln_context_replace_ops(ctx, lp, 1, ep_ops);
        }
    }
}

void ln_pass_combiner(ln_context *ctx, size_t win_size,
                      const ln_combiner_func *cb_funcs)
{
    ln_combiner_func cb;
    ln_list *win_out;
    ln_list **lp;
    int stable = 0;
    int count = 0;
    int match;
    int i;

    while (!stable) {
        stable = 1;
        for (lp = &ctx->ops; *lp; lp = &(*lp)->next) {
            if (ln_list_length(*lp) < win_size)
                break;
            for (i = 0; (cb = cb_funcs[i]); i++) {
                match = 0;
                win_out = cb(*lp, win_size, ctx->dfg, &match);
                if (!match)
                    continue;
                stable = 0;
                ln_context_replace_ops(ctx, lp, win_size, win_out);
            }
        }
        count++;
        if (count > MAX_PEEPHOLE_PASSES) {
            ln_error_emit(LN_INTER_WARNING,
                          "peephole passes exceeds limit of %d",
                          MAX_PEEPHOLE_PASSES);
        }
    }
}

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

ln_list *ln_pass_expand(ln_list *ops, ln_hash *op_table,
                        ln_expander_func *ep_funcs)
{
    ln_op *op;
    ln_list *l;
    ln_list *new_ops;
    ln_list *ep_ops;
    ln_graph *DFG;
    ln_hash *node_table;
    ln_expander_func ep_func;
    int i;

    new_ops = NULL;
    ep_ops = NULL;
    DFG = ln_op_list_gen_DFG(ops, &node_table);
    for (l = ops; l; l = l->next) {
        op = l->data;
        for (i = 0; (ep_func = ep_funcs[i]); i++) {
            ep_ops = ep_func(op, op_table, node_table);
        }
    }

    ln_graph_free(DFG);
    ln_hash_free(node_table);

    return new_ops;
}

static inline void ph_error_handle(ln_error **error)
{
    if (*error) {
        fprintf(stderr, "Infomation generated in ln_pass_peephole():\n");
        ln_error_handle(error);
    }
}

ln_list *ln_pass_peephole(ln_list *ops, int win_size, ln_peephole_func *ph_funcs,
                          ln_post_peephole post_ph, ln_hash *op_table)
{
    ln_peephole_func pf;
    ln_op *op;
    ln_list *win_in, *win_out;
    ln_list *l_in, *l_out, *l_ops, *l_ops_pre, *l;
    int stable = 0;
    int match;
    int i;                     /* actual window size */
    int j;                     /* peephole function index */
    ln_error *error = NULL;

    while (!stable) {
        stable = 1;
        l_ops_pre = NULL;
        for (l_ops = ops; l_ops; l_ops_pre = l_ops, l_ops = l_ops->next) {
            win_in = NULL;
            for (i = 0, l = l_ops;
                 i < win_size && l && !((ln_op *)l->data)->op_arg->fixed;
                 i++, l = l->next)
                win_in = ln_list_append(win_in, l->data);
            if (i == 0)
                continue;
            for (j = 0; (pf = ph_funcs[j]); j++) {
                win_out = pf(win_in, i, &match, ops);
                if (!match)
                    continue;
                stable = 0;
                for (l = l_ops, l_in = win_in, l_out = win_out;
                     l_in && l_out;
                     l = l->next, l_in = l_in->next, l_out = l_out->next) {
                    op = l->data;
                    op->post_run(op->op_arg, &error);
                    ph_error_handle(&error);
                    ln_op_table_remove(op_table, op->op_arg->name);
                    l->data = l_out->data;
                    op = l->data;
                    ln_op_table_insert(op_table, op->op_arg->name, op);
                    op->pre_run(op->op_arg, &error);
                    ph_error_handle(&error);
                }
                if (!l_in && l_out) {
                    for (; l_out; l_out = l_out->next) {
                        l_ops = ln_list_insert_before(l_ops, l_out->data, l);
                        op = l_out->data;
                        ln_op_table_insert(op_table, op->op_arg->name, op);
                        op->pre_run(op->op_arg, &error);
                        ph_error_handle(&error);
                    }
                } else if (l_in && !l_out) {
                    for (; l_in; l_in = l_in->next) {
                        op = l_in->data;
                        op->post_run(op->op_arg, &error);
                        ph_error_handle(&error);
                        l_ops = ln_list_remove(l_ops, op);
                        ln_op_table_remove(op_table, op->op_arg->name);
                        if (!l_ops_pre)
                            ops = l_ops;
                        else
                            l_ops_pre->next = l_ops;
                    }
                } else if (l_in && l_out) {
                    assert(0 && "shouldn't get here");
                }
                ln_list_free(win_out);
                break;
            }
            ln_list_free(win_in);
        }
    }
    if (post_ph)
        ops = post_ph(ops);

    return ops;
}

static inline void use_count_zero(ln_hash *use_counts, char *name)
{
    ln_hash_insert(use_counts, name, (void *)0);
}

static inline ssize_t use_count_inc(ln_hash *use_counts, char *name)
{
    int found;
    ssize_t uc;

    found = ln_hash_find_extended(use_counts, name, NULL, (void **)&uc);
    assert(found);
    ln_hash_insert(use_counts, name, (void *)(++uc));
    return uc;
}

static inline ssize_t use_count_dec(ln_hash *use_counts, char *name)
{
    int found;
    ssize_t uc;

    found = ln_hash_find_extended(use_counts, name, NULL, (void **)&uc);
    assert(found);
    ln_hash_insert(use_counts, name, (void *)(--uc));
    assert(uc >= 0);
    return uc;
}

static inline ssize_t use_count_of(ln_hash *use_counts, char *name)
{
    int found;
    ssize_t uc;

    found = ln_hash_find_extended(use_counts, name, NULL, (void **)&uc);
    assert(found);
    return uc;
}

ln_list *ln_pass_mem(ln_list *ops, ln_hash *mem_pools)
{
    ln_op *op;
    ln_op_arg *arg;
    ln_hash *use_counts;
    ln_tensor_entry *te;
    ln_tensor_list_entry *tle;
    ln_mem_pool *mp;
    ln_list *unused_tles;

    use_counts = ln_hash_create(ln_str_hash, ln_str_cmp, NULL, NULL);
    LN_LIST_FOREACH(op, ops) {
        arg = op->op_arg;
        LN_LIST_FOREACH(tle, arg->tensors_out) {
            te = ln_tensor_table_find(arg->tensor_table, tle->name);
            mp = ln_hash_find(mem_pools, (void *)te->mtype);
            if (te->owner)
                continue;
            if (te->isstatic) {
                te->offset = ln_mem_alloc(mp, tl_tensor_size(te->tensor));
                continue;
            }
            if (ln_hash_find_extended(use_counts, te->name, NULL, NULL))
                use_count_inc(use_counts, te->name);
            else
                use_count_zero(use_counts, te->name);
        }
        LN_LIST_FOREACH(tle, arg->tensors_in) {
            te = ln_tensor_table_find(arg->tensor_table, tle->name);
            if (te->owner) {
                use_count_inc(use_counts, te->owner);
                continue;
            }
            if (te->isstatic)
                continue;
            use_count_inc(use_counts, te->name);
        }
    }

    LN_LIST_FOREACH(op, ops) {
        arg = op->op_arg;
        unused_tles = NULL;
        LN_LIST_FOREACH(tle, arg->tensors_out) {
            te = ln_tensor_table_find(arg->tensor_table, tle->name);
            mp = ln_hash_find(mem_pools, (void *)te->mtype);
            if (te->owner)
                continue;
            if (te->isstatic)
                continue;
            if (ln_mem_exist(mp, te->offset)) {
                use_count_dec(use_counts, te->name);
            }
            else {
                te->offset = ln_mem_alloc(mp, tl_tensor_size(te->tensor));
            }
            if (use_count_of(use_counts, te->name) == 0)
                unused_tles = ln_list_prepend(unused_tles, tle);
        }
        LN_LIST_FOREACH(tle, unused_tles) {
            te = ln_tensor_table_find(arg->tensor_table, tle->name);
            mp = ln_hash_find(mem_pools, (void *)te->mtype);
            ln_mem_free(mp, te->offset);
        }
        ln_list_free(unused_tles);
        LN_LIST_FOREACH(tle, arg->tensors_in) {
            te = ln_tensor_table_find(arg->tensor_table, tle->name);
            mp = ln_hash_find(mem_pools, (void *)te->mtype);
            if (te->owner) {
                if (use_count_dec(use_counts, te->owner) == 0) {
                    te = ln_tensor_table_find(arg->tensor_table, te->owner);
                    ln_mem_free(mp, te->offset);
                }
                continue;
            }
            if (te->isstatic)
                continue;
            if (use_count_dec(use_counts, te->name) == 0) {
                ln_mem_free(mp, te->offset);
            }
        }
    }

    ln_hash_free(use_counts);
    return ops;
}

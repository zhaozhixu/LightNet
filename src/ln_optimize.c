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
#include "ln_optimize.h"

static inline void use_count_zero(ln_hash *use_counts, char *name)
{
     ln_hash_insert(use_counts, name, (void *)0);
}

static inline ssize_t use_count_inc(ln_hash *use_counts, char *name)
{
     int found;
     ssize_t uc;

     found = ln_hash_find_extended(use_counts, name, (void **)&uc);
     assert(found);
     ln_hash_insert(use_counts, name, (void *)(uc+1));
     return uc;
}

static inline ssize_t use_count_dec(ln_hash *use_counts, char *name)
{
     int found;
     ssize_t uc;

     found = ln_hash_find_extended(use_counts, name, (void **)&uc);
     assert(found);
     ln_hash_insert(use_counts, name, (void *)(uc-1));
     assert(uc >= 0);
     return uc;
}

static inline ssize_t use_count_of(ln_hash *use_counts, char *name)
{
     int found;
     ssize_t uc;

     found = ln_hash_find_extended(use_counts, name, (void **)&uc);
     assert(found);
     return uc;
}

ln_list *ln_optimize_mem(ln_list *ops, ln_hash *mem_pools)
{
     ln_op *op;
     ln_hash *use_counts;
     ln_tensor_entry *te;
     tl_tensor *t;
     ln_mem_pool *mp;
     ln_list *unused_tes;

     use_counts = ln_hash_create(ln_str_hash, ln_str_cmp, NULL, NULL);
     LN_LIST_FOREACH(op, ops) {
          LN_LIST_FOREACH(te, op->op_arg->tensors_out) {
               if (ln_hash_find_extended(use_counts, te->name, NULL))
                    use_count_inc(use_counts, te->name);
               else
                    use_count_zero(use_counts, te->name);
          }
          LN_LIST_FOREACH(te, op->op_arg->tensors_in) {
               use_count_inc(use_counts, te->name);
          }
     }

     LN_LIST_FOREACH(op, ops) {
          unused_tes = NULL;
          LN_LIST_FOREACH(te, op->op_arg->tensors_out) {
               mp = ln_hash_find(mem_pools, (void *)te->mtype);
               t = te->tensor;
               if (ln_mem_exist(mp, (size_t)t->data)) {
                    use_count_dec(use_counts, te->name);
               } else {
                    t->data = (void *)ln_mem_alloc(mp, tl_tensor_size(t));
               }
               if (use_count_of(use_counts, te->name) == 0)
                    unused_tes = ln_list_prepend(unused_tes, te);
          }
          LN_LIST_FOREACH(te, op->op_arg->tensors_in) {
               if (use_count_dec(use_counts, te->name) == 0) {
                    mp = ln_hash_find(mem_pools, (void *)te->mtype);
                    ln_mem_free(mp, (size_t)te->tensor->data);
               }
          }
          LN_LIST_FOREACH(te, unused_tes) {
               mp = ln_hash_find(mem_pools, (void *)te->mtype);
               ln_mem_free(mp, (size_t)te->tensor->data);
          }
          ln_list_free(unused_tes);
     }

     ln_hash_free(use_counts);
     return ops;
}

ln_list *ln_optimize_mtype(ln_list *ops, ln_mem_type mtype)
{


     return ops;
}

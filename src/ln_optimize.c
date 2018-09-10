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

     found = ln_hash_find_extended(use_counts, name, NULL, (void **)&uc);
     assert(found);
     ln_hash_insert(use_counts, name, (void *)(uc+1));
     return uc;
}

static inline ssize_t use_count_dec(ln_hash *use_counts, char *name)
{
     int found;
     ssize_t uc;

     found = ln_hash_find_extended(use_counts, name, NULL, (void **)&uc);
     assert(found);
     ln_hash_insert(use_counts, name, (void *)(uc-1));
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

ln_list *ln_optimize_mem(ln_list *ops, ln_hash *mem_pools)
{
     ln_op *op;
     ln_op_arg *arg;
     ln_hash *use_counts;
     ln_tensor_entry *te;
     ln_mem_pool *mp;
     ln_list *unused_tes;

     use_counts = ln_hash_create(ln_str_hash, ln_str_cmp, NULL, NULL);
     LN_LIST_FOREACH(op, ops) {
          arg = op->op_arg;
          mp = ln_hash_find(mem_pools, (void *)arg->mtype_out);
          LN_LIST_FOREACH(te, arg->tensors_out) {
               te = ln_tensor_table_find(arg->tensor_table, te->name);
               if (te->tensor->data)
                    te->offset = ln_mem_alloc(mp, tl_tensor_size(te->tensor));
               if (ln_hash_find_extended(use_counts, te->name, NULL, NULL))
                    use_count_inc(use_counts, te->name);
               else
                    use_count_zero(use_counts, te->name);
          }
          LN_LIST_FOREACH(te, arg->tensors_in) {
               use_count_inc(use_counts, te->name);
          }
     }

     LN_LIST_FOREACH(op, ops) {
          arg = op->op_arg;
          unused_tes = NULL;
          mp = ln_hash_find(mem_pools, (void *)arg->mtype_out);
          LN_LIST_FOREACH(te, arg->tensors_out) {
               te = ln_tensor_table_find(arg->tensor_table, te->name);
               if (ln_mem_exist(mp, te->offset)) {
                    use_count_dec(use_counts, te->name);
               } else {
                    te->offset = ln_mem_alloc(mp, tl_tensor_size(te->tensor));
               }
               if (use_count_of(use_counts, te->name) == 0)
                    unused_tes = ln_list_prepend(unused_tes, te);
          }
          LN_LIST_FOREACH(te, unused_tes) {
               te = ln_tensor_table_find(arg->tensor_table, te->name);
               ln_mem_free(mp, te->offset);
          }
          ln_list_free(unused_tes);
          mp = ln_hash_find(mem_pools, (void *)arg->mtype_in);
          LN_LIST_FOREACH(te, arg->tensors_in) {
               te = ln_tensor_table_find(arg->tensor_table, te->name);
               if (use_count_dec(use_counts, te->name) == 0) {
                    ln_mem_free(mp, te->offset);
               }
          }
     }

     ln_hash_free(use_counts);
     return ops;
}

ln_list *ln_optimize_mtype(ln_list *ops, ln_mem_type mtype)
{


     return ops;
}

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

#ifndef _LN_CONTEXT_H_
#define _LN_CONTEXT_H_

#include "ln_list.h"
#include "ln_hash.h"
#include "ln_mem.h"
#include "ln_tensor.h"
#include "ln_op.h"
#include "ln_dfg.h"

struct ln_context {
    ln_hash *tensor_table;
    ln_hash *op_table;
    ln_dfg  *dfg;
    ln_list *ops;
    void    *mem_starts[LN_MEM_TYPE_SIZE];
    size_t   mem_sizes[LN_MEM_TYPE_SIZE];
};
typedef struct ln_context ln_context;

#ifdef __cplusplus
LN_CPPSTART
#endif

ln_context *ln_context_create(void);
void ln_context_free(ln_context *ctx);
void ln_context_init_ops(ln_context *ctx);
void ln_context_cleanup_ops(ln_context *ctx);
void ln_context_replace_ops(ln_context *ctx, ln_list **position,
                            size_t len, ln_list *new_ops);
void ln_context_remove_op(ln_context *ctx, ln_list **positio);
void ln_context_add_op(ln_context *ctx, ln_list **position, ln_op *new_op);
int ln_context_check(const ln_context *ctx);
void ln_context_alloc_mem(ln_context *ctx);
void ln_context_dealloc_mem(ln_context *ctx);

void ln_context_init(ln_context *ctx, const char *source);
void ln_context_compile(ln_context *ctx, const char *target);
void ln_context_print(const ln_context *ctx, const char *outfile);
void ln_context_load(ln_context *ctx, const char *datafile);
void ln_context_set_data(ln_context *ctx, const char *tname, const void *data);
void *ln_context_get_data(ln_context *ctx, const char *tname, void *data);
size_t ln_context_data_size(ln_context *ctx, const char *name);
void ln_context_set_param(ln_context *ctx, const char *opname,
                          const char *pname, ...);
void ln_context_run(const ln_context *ctx);
void ln_context_unload(ln_context *ctx);
void ln_context_cleanup(ln_context *ctx);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif  /* _LN_CONTEXT_H_ */

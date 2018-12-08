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

#ifndef _LN_ARCH_H_
#define _LN_ARCH_H_

#include "ln_op.h"
#include "ln_dfg.h"

typedef ln_list *(*ln_expander_func) (const ln_op *op, const ln_dfg *dfg,
                                    int *match);
typedef ln_list *(*ln_combiner_func) (const ln_list *ops, size_t size,
                                     const ln_dfg *dfg, int *match);

struct ln_arch {
    ln_op             **reg_ops;        /* NULL terminated */
    void (*init_func)(void);
    void (*cleanup_func)(void);
    ln_expander_func    *ep_funcs;      /* NULL terminated */
    ln_combiner_func    *cb_funcs;      /* NULL terminated */
    char                *arch_name;
};
typedef struct ln_arch ln_arch;

struct ln_arch_tables {
    ln_hash *arch_table;
    ln_hash *op_proto_table;
};
typedef struct ln_arch_tables ln_arch_tables;

extern ln_arch_tables ln_global_arch_tables;
#define LN_ARCH ln_global_arch_tables

#ifdef __cplusplus
LN_CPPSTART
#endif

void ln_arch_init(void);
void ln_arch_cleanup(void);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif  /* _LN_ARCH_H_ */

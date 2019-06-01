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

#ifndef _LN_DFG_H_
#define _LN_DFG_H_

#include "ln_op.h"
#include "ln_graph.h"
#include "ln_hash.h"

struct ln_dfg {
    ln_graph *graph;
    ln_hash  *node_table;
    ln_list  *dangling_ins;
    ln_list  *dangling_outs;
};
typedef struct ln_dfg ln_dfg;

#ifdef __cplusplus
LN_CPPSTART
#endif

/* Generate Data Flow Graph, with ops as its nodes and tensor names
   as its edge. */
ln_dfg *ln_dfg_create(void);
void ln_dfg_free(ln_dfg *dfg);asdf
void ln_dfg_link(ln_dfg *dfg, ln_op *op1, ln_op *op2, const char *tname);
void ln_dfg_unlink(ln_dfg *dfg, ln_op *op1, ln_op *op2, const char *tname);
void ln_dfg_add(ln_dfg *dfg, ln_op *op);
void ln_dfg_remove(ln_dfg *dfg, ln_op *op);
ln_op *ln_dfg_next(const ln_dfg *dfg, const ln_op *op, const char *tname);
ln_list *ln_dfg_nexts(const ln_dfg *dfg, const ln_op *op, const char *tname);
ln_op *ln_dfg_prev(const ln_dfg *dfg, const ln_op *op, const char *tname);
int ln_dfg_check(const ln_dfg *dfg);
void ln_dfg_fprint(FILE *fp, const ln_dfg *dfg);
void ln_dfg_print(const ln_dfg *dfg);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif  /* _LN_DFG_H_ */

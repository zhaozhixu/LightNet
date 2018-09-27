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

#ifndef _LN_PASS_H_
#define _LN_PASS_H_

#include "ln_list.h"
#include "ln_hash.h"
#include "ln_mem.h"
#include "ln_op.h"
#include "ln_arch.h"

#ifdef __cplusplus
LN_CPPSTART
#endif

ln_list *ln_pass_mem(ln_list *ops, ln_hash *mem_pools);
ln_list *ln_pass_peephole(ln_list *ops, ln_peephole_func *ph_funcs);
ln_list *ln_pass_parse(const char *json_str, ln_list *registered_ops,
                       ln_hash *tensor_table);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif  /* _LN_OPTIMIZE_H_ */

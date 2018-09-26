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

#include "ln_mem.h"
#include "ln_op.h"

typedef enum ln_arch_type ln_arch_type;
enum ln_arch_type {
     LN_ARCH_CPU,
     LN_ARCH_CUDA,
};

typedef ln_list *(*ln_peephole_func) (ln_list *ops);

typedef struct ln_arch ln_arch;
struct ln_arch {
     ln_op            **ops;       /* NULL terminated */
     ln_peephole_func **ph_funcs;  /* NULL terminated */
     ln_arch_type       atype;
};

#endif  /* _LN_ARCH_H_ */

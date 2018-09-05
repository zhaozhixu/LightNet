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

#ifndef _LN_MEM_H_
#define _LN_MEM_H_

#include <stdlib.h>
#include "ln_list.h"

typedef enum ln_mem_type ln_mem_type;
enum ln_mem_type {
     LN_MEM_UNDEFINED,
     LN_MEM_CPU,
     LN_MEM_CUDA
};

typedef struct ln_mem_pool ln_mem_pool;
struct ln_mem_pool {
     size_t   size;
     size_t   align_size;
     ln_list *mem_blocks;
};

#ifdef __cplusplus
LN_CPPSTART
#endif

ln_mem_pool *ln_mem_pool_create(size_t size, size_t align_size);
void ln_mem_pool_free(ln_mem_pool *mem_pool);
size_t ln_mem_alloc(ln_mem_pool *mem_pool, size_t size);
void ln_mem_free(ln_mem_pool *mem_pool, size_t addr);
int ln_mem_exist(ln_mem_pool *mem_pool, size_t addr);
void ln_mem_dump(ln_mem_pool *mem_pool, FILE *fp);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif  /* _LN_MEM_H_ */

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

#ifndef _LN_MEM_H_
#define _LN_MEM_H_

#include <stdlib.h>
#include "ln_list.h"
#include "ln_hash.h"

/* NOTE: keep this sync with mtype_infos in ln_mem.c */
enum ln_mem_type {
    LN_MEM_NONE = 0,
    LN_MEM_CPU,
#ifdef LN_CUDA
    LN_MEM_CUDA,
#endif
    LN_MEM_TYPE_SIZE
};
typedef enum ln_mem_type ln_mem_type;

typedef struct ln_mem_pool ln_mem_pool;

struct ln_mem_info {
    const char  *name;
    void      *(*alloc_func)(size_t n);
    void       (*free_func)(void *p);
    void      *(*memset_func)(void *s, int c, size_t n);
    size_t       max_size;
    size_t       align_size;
};
typedef struct ln_mem_info ln_mem_info;

#ifdef __cplusplus
LN_CPPSTART
#endif

const char *ln_mem_type_name(ln_mem_type mtype);
const ln_mem_info ln_mem_type_info(ln_mem_type mtype);
ln_copy_func ln_mem_type_copy_func(ln_mem_type dst_mtype, ln_mem_type src_mtype);

ln_mem_pool *ln_mem_pool_create(size_t size, size_t align_size);
void ln_mem_pool_free(ln_mem_pool *mem_pool);
size_t ln_mem_pool_alloc(ln_mem_pool *mem_pool, size_t size);
void ln_mem_pool_dealloc(ln_mem_pool *mem_pool, size_t addr);
int ln_mem_pool_exist(ln_mem_pool *mem_pool, size_t addr);
void ln_mem_pool_dump(ln_mem_pool *mem_pool, FILE *fp);
ln_hash *ln_mem_pool_table_create(void);
void ln_mem_pool_table_free(ln_hash *mpt);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif  /* _LN_MEM_H_ */

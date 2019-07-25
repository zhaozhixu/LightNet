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

#include <assert.h>
#include "ln_mem.h"
#include "ln_msg.h"
#include "ln_cuda.h"
#include "ln_dpu.h"

typedef enum block_flag block_flag;
enum block_flag {
    HOLE,
    SYMBOL
};

typedef struct mem_block mem_block;
struct mem_block {
    block_flag flag;
    size_t   start;
    size_t   size;
};

typedef struct ln_mem_pool ln_mem_pool;
struct ln_mem_pool {
    size_t   size;
    size_t   align_size;
    ln_list *mem_blocks;
};

#define DEFAULT_MAX_SIZE 17179869184 /* 16GB */
#define DEFAULT_ALIGN_SIZE 1

static const ln_mem_info ln_mem_infos[] = {
    {"LN_MEM_NONE", NULL, NULL, NULL, 0, 0},
    {"LN_MEM_CPU", ln_alloc, ln_free, memset,
     DEFAULT_MAX_SIZE, DEFAULT_ALIGN_SIZE},
#ifdef LN_CUDA
    {"LN_MEM_CUDA", ln_alloc_cuda, ln_free_cuda, ln_memset_cuda,
     DEFAULT_MAX_SIZE, 32}, /* 32-byte L2 cache in compute capability >= 3.0*/
#endif
#ifdef LN_DPU
    {"LN_MEM_DPU", ln_alloc_dpu, ln_free_dpu, ln_memset_dpu,
     524288, 1}, /* 512x64x16 bytes in MRA and MRB */
#endif
    {NULL, NULL, NULL, NULL, 0, 0},
};

#define ln_check_mem_type(mtype)                        \
    assert(mtype >= 0 && mtype < LN_MEM_TYPE_SIZE)

const char *ln_mem_type_name(ln_mem_type mtype)
{
    return ln_mem_type_info(mtype).name;
}

const ln_mem_info ln_mem_type_info(ln_mem_type mtype)
{
    ln_mem_info minfo;

    ln_check_mem_type(mtype);
    minfo = ln_mem_infos[mtype];
    if (!minfo.align_size && !minfo.max_size &&
        !minfo.alloc_func && !minfo.free_func)
        ln_msg_error("unsupported ln_mem_type %s", ln_mem_type_name(mtype));

    return minfo;
}

#define MEM_COPY_ERROR ln_msg_error("unsupported copy direction: %s -> %s", \
                                    ln_mem_type_name(src_mtype),        \
                                    ln_mem_type_name(dst_mtype))

/* TODO: re-implement it with a 2d-array */
ln_copy_func ln_mem_type_copy_func(ln_mem_type dst_mtype, ln_mem_type src_mtype)
{
    ln_copy_func copy = NULL;

    ln_check_mem_type(dst_mtype);
    ln_check_mem_type(src_mtype);

    switch (dst_mtype) {
    case LN_MEM_CPU:
        switch (src_mtype) {
        case LN_MEM_CPU:
            copy = memmove;
            break;
#ifdef LN_CUDA
        case LN_MEM_CUDA:
            copy = ln_memcpy_d2h;
            break;
#endif  /* LN_CUDA */
        default:
            MEM_COPY_ERROR;
            break;
        }
        break;
#ifdef LN_CUDA
    case LN_MEM_CUDA:
        switch (src_mtype) {
        case LN_MEM_CPU:
            copy = ln_memcpy_h2d;
            break;
        case LN_MEM_CUDA:
            copy = ln_memcpy_d2d;
            break;
        default:
            MEM_COPY_ERROR;
            break;
        }
        break;
#endif  /* LN_CUDA */
    default:
        MEM_COPY_ERROR;
        break;
    }
    return copy;
}

static mem_block *mem_block_create(block_flag flag, size_t start, size_t size)
{
    mem_block *block;
    block = ln_alloc(sizeof(mem_block));
    block->flag = flag;
    block->start = start;
    block->size = size;

    return block;
}

static void mem_block_free(mem_block *block)
{
    ln_free(block);
}

ln_mem_pool *ln_mem_pool_create(size_t size, size_t align_size)
{
    ln_mem_pool *mem_pool;
    mem_block *block;

    assert(size > 0 && align_size > 0);
    mem_pool = ln_alloc(sizeof(ln_mem_pool));
    mem_pool->size = size;
    mem_pool->align_size = align_size;
    block = mem_block_create(HOLE, 1, size);
    mem_pool->mem_blocks = ln_list_append(NULL, block);

    return mem_pool;
}

static void mem_block_free_wrapper(void *block)
{
    mem_block_free((mem_block *)block);
}

void ln_mem_pool_free(ln_mem_pool *mem_pool)
{
    ln_list_free_deep(mem_pool->mem_blocks, mem_block_free_wrapper);
    ln_free(mem_pool);
}

/* static int first_fit(ln_mem_pool *mem_pool, size_t size) */
/* { */
/*     size_t align_size = mem_pool->align_size; */
/*     ln_list *l; */
/*     int i; */

/*     for (l = mem_pool->mem_blocks, i = 0; l; l = l->next, i++) { */
/*         mem_block *block = l->data; */
/*         size_t align_start = block->start % align_size == 0 ? block->start : */
/*             align_size - block->start % align_size + block->start; */
/*         size_t mem_end = block->start + block->size - 1; */
/*         if (block->flag == HOLE && align_start + size - 1 <= mem_end) */
/*             return i; */
/*     } */
/*     return -1; */
/* } */

static int best_fit(ln_mem_pool *mem_pool, size_t size)
{
    int first_hole = 0;
    size_t min_size = SIZE_MAX;
    size_t align_size = mem_pool->align_size;
    int min_idx = -1;
    ln_list *l;
    int i;

    for (l = mem_pool->mem_blocks, i = 0; l; l = l->next, i++) {
        mem_block *block = l->data;
        size_t align_start = block->start % align_size == 0 ? block->start :
            align_size - block->start % align_size + block->start;
        size_t mem_end = block->start + block->size - 1;
        if (block->flag != HOLE || align_start + size - 1 > mem_end)
            continue;
        size_t alignable_size = mem_end - align_start + 1;
        if (!first_hole) {
            first_hole = 1;
            min_size = alignable_size;
            min_idx = i;
            continue;
        }
        if (alignable_size < min_size) {
            min_size = alignable_size;
            min_idx = i;
        }
    }
    if (!first_hole)
        return -1;
    return min_idx;
}

size_t ln_mem_pool_alloc(ln_mem_pool *mem_pool, size_t size)
{
    assert(size > 0);
    int fit_idx = best_fit(mem_pool, size);
    if (fit_idx < 0)
        ln_msg_error("ln_mem_pool_alloc(): out of virtual memory pool when allocating %ld bytes",
                     size);

    mem_block *block = ln_list_nth_data(mem_pool->mem_blocks, fit_idx);
    size_t align_size = mem_pool->align_size;
    size_t align_start = block->start % align_size == 0 ? block->start :
        align_size - block->start % align_size + block->start;
    size_t mem_size = size + align_start - block->start;
    mem_block *new_block = mem_block_create(SYMBOL, block->start, mem_size);
    mem_pool->mem_blocks = ln_list_insert_nth(mem_pool->mem_blocks,
                                              new_block, fit_idx);
    block->start += mem_size;
    block->size -= mem_size;
    if (block->size == 0)
        mem_pool->mem_blocks = ln_list_remove_nth_deep(mem_pool->mem_blocks,
                                                       fit_idx + 1,
                                                       mem_block_free_wrapper);

    size_t hole_size = align_start - new_block->start;
    if (hole_size == 0)
        return align_start;
    mem_block *hole_block = mem_block_create(HOLE, new_block->start, hole_size);
    mem_pool->mem_blocks = ln_list_insert_nth(mem_pool->mem_blocks,
                                              hole_block, fit_idx);
    new_block->start = align_start;
    new_block->size -= hole_size;
    return align_start;
}

void ln_mem_pool_dealloc(ln_mem_pool *mem_pool, size_t addr)
{
    int i;
    ln_list *l, *l_next, *l_before = NULL;
    mem_block *block;
    for (l = mem_pool->mem_blocks, i = 0; l; l_before = l, l = l->next, i++) {
        block = l->data;
        if (block->flag != SYMBOL || block->start != addr)
            continue;
        l_next = l->next;
        if (l_next && ((mem_block *)l_next->data)->flag == HOLE) {
            block->size += ((mem_block *)l_next->data)->size;
            mem_pool->mem_blocks = ln_list_remove_nth_deep(mem_pool->mem_blocks,
                                                           i + 1,
                                                           mem_block_free_wrapper);
        }
        if (l_before && ((mem_block *)l_before->data)->flag == HOLE) {
            block->start -= ((mem_block *)l_before->data)->size;
            block->size += ((mem_block *)l_before->data)->size;
            mem_pool->mem_blocks = ln_list_remove_nth_deep(mem_pool->mem_blocks,
                                                           i - 1,
                                                           mem_block_free_wrapper);
        }
        block->flag = HOLE;
        break;
    }
    if (!l)
        ln_msg_error("ln_mem_pool_dealloc(): invalid address: 0x%012lx",
                     addr);
}

int ln_mem_pool_exist(ln_mem_pool *mem_pool, size_t addr)
{
    mem_block *block;

    LN_LIST_FOREACH(block, mem_pool->mem_blocks) {
        if (block->flag == SYMBOL && block->start == addr)
            return 1;
    }
    return 0;
}

void ln_mem_pool_dump(ln_mem_pool *mem_pool, FILE *fp)
{
    mem_block *block;

    fprintf(fp, "======= Lightnet Memory Plan Map: =======\n");
    LN_LIST_FOREACH(block, mem_pool->mem_blocks) {
        fprintf(fp, "0x%012lx-0x%012lx %s\n", block->start,
                block->start+block->size-1, block->flag==HOLE?"H":"S");
    }
}

static void mem_pool_free_wrapper(void *p)
{
    ln_mem_pool_free(p);
}

ln_hash *ln_mem_pool_table_create(void)
{
    ln_hash *mpt;
    ln_mem_pool *mp;
    int i;

    mpt = ln_hash_create(ln_direct_hash, ln_direct_cmp, NULL,
                         mem_pool_free_wrapper);
    for (i = LN_MEM_NONE+1; i < LN_MEM_TYPE_SIZE; i++) {
        mp = ln_mem_pool_create(ln_mem_infos[i].max_size,
                                ln_mem_infos[i].align_size);
        ln_hash_insert(mpt, (void *)(size_t)i, mp);
    }
    return mpt;
}

void ln_mem_pool_table_free(ln_hash *mpt)
{
    ln_hash_free(mpt);
}

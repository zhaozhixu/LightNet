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
#include "ln_mem.h"
#include "ln_error.h"

typedef enum mem_flag mem_flag;
enum mem_flag {
     HOLE,
     SYMBOL
};

typedef struct mem_info mem_info;
struct mem_info {
     mem_flag flag;
     size_t   start;
     size_t   size;
};

static mem_info *mem_info_create(mem_flag flag, size_t start, size_t size)
{
     mem_info *minfo;
     minfo = ln_alloc(sizeof(mem_info));
     minfo->flag = flag;
     minfo->start = start;
     minfo->size = size;

     return minfo;
}

static void mem_info_free(mem_info *minfo)
{
     ln_free(minfo);
}

ln_mem_pool *ln_mem_pool_create(size_t size, size_t align_size)
{
     ln_mem_pool *mem_pool;
     mem_info *minfo;

     assert(size > 0 && align_size > 0);
     mem_pool = ln_alloc(sizeof(ln_mem_pool));
     mem_pool->size = size;
     mem_pool->align_size = align_size;
     minfo = mem_info_create(HOLE, 0, size);
     mem_pool->mem_blocks = ln_list_create();
     ln_list_append(mem_pool->mem_blocks, minfo);

     return mem_pool;
}

static void mem_info_free_wrapper(void *minfo)
{
     mem_info_free((mem_info *)minfo);
}

void ln_mem_pool_free(ln_mem_pool *mem_pool)
{
     ln_list_free_deep(mem_pool->mem_blocks, mem_info_free_wrapper);
     ln_free(mem_pool);
}

static int best_fit(ln_mem_pool *mem_pool, size_t size)
{
     int first_hole = 0;
     size_t min_size;
     size_t align_size = mem_pool->align_size;
     int min_idx;
     ln_list_node *ln;
     int i;

     for (ln = mem_pool->mem_blocks->head, i = 0; ln; ln = ln->next, i++) {
          mem_info *minfo = ln->data;
          size_t align_start = minfo->start % align_size == 0 ? minfo->start :
               align_size - minfo->start % align_size + minfo->start;
          size_t mem_end = minfo->start + minfo->size - 1;
          if (minfo->flag != HOLE || align_start + size - 1 > mem_end)
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

size_t ln_mem_alloc(ln_mem_pool *mem_pool, size_t size)
{
     assert(size > 0);
     int fit_idx = best_fit(mem_pool, size);
     if (fit_idx < 0) {
          ln_error *error = ln_error_create(LN_ERROR,
                                            "ln_mem_alloc(): out of virtual memory pool when allocating %ld bytes", size);
          ln_error_handle(&error);
     }

     mem_info *minfo = ln_list_nth_data(mem_pool->mem_blocks, fit_idx);
     size_t align_size = mem_pool->align_size;
     size_t align_start = minfo->start % align_size == 0 ? minfo->start :
          align_size - minfo->start % align_size + minfo->start;
     size_t mem_size = size + align_start - minfo->start;
     mem_info *new_minfo = mem_info_create(SYMBOL, minfo->start, mem_size);
     ln_list_insert_nth(mem_pool->mem_blocks, new_minfo, fit_idx);
     minfo->start += mem_size;
     minfo->size -= mem_size;
     if (minfo->size == 0)
          ln_list_remove_nth_deep(mem_pool->mem_blocks, fit_idx + 1,
                                  mem_info_free_wrapper);

     size_t hole_size = align_start - new_minfo->start;
     if (hole_size == 0)
          return align_start;
     mem_info *hole_minfo = mem_info_create(HOLE, new_minfo->start, hole_size);
     ln_list_insert_nth(mem_pool->mem_blocks, hole_minfo, fit_idx);
     new_minfo->start = align_start;
     new_minfo->size -= hole_size;
     best_fit(mem_pool, 8);
     return align_start;
}

void ln_mem_free(ln_mem_pool *mem_pool, size_t addr)
{
     int i;
     ln_list_node *ln, *ln_next, *ln_before = NULL;
     mem_info *minfo;
     for (ln = mem_pool->mem_blocks->head, i = 0; ln;
          ln_before = ln, ln = ln->next, i++) {
          minfo = ln->data;
          if (minfo->flag != SYMBOL || minfo->start != addr)
               continue;
          ln_next = ln->next;
          if (ln_next && ((mem_info *)ln_next->data)->flag == HOLE) {
               minfo->size += ((mem_info *)ln_next->data)->size;
               ln_list_remove_nth_deep(mem_pool->mem_blocks, i + 1,
                                       mem_info_free_wrapper);
          }
          if (ln_before && ((mem_info *)ln_before->data)->flag == HOLE) {
               minfo->start -= ((mem_info *)ln_before->data)->size;
               minfo->size += ((mem_info *)ln_before->data)->size;
               ln_list_remove_nth_deep(mem_pool->mem_blocks, i - 1,
                                       mem_info_free_wrapper);
          }
          minfo->flag = HOLE;
          break;
     }
     if (!ln) {
          ln_error *error = ln_error_create(LN_ERROR,
                                            "ln_mem_free(): invalid address: 0x%012lx",
                                            addr);
          ln_error_handle(&error);
     }
}

void ln_mem_dump(ln_mem_pool *mem_pool, FILE *fp)
{
     mem_info *minfo;

     LN_LIST_FOR_EACH(minfo, mem_pool->mem_blocks) {
          fprintf(fp, "0x%012lx-0x%012lx %s\n", minfo->start,
                  minfo->start+minfo->size-1, minfo->flag==HOLE?"H":"S");
     }
}

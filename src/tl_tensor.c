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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "tl_tensor.h"
#include "tl_util.h"

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

static inline int get_index(int *ids, int ndim, int *dims)
{
     int i, id;
     for (i = 0, id = ids[0]; i < ndim-1; i++)
          id = dims[i+1] * id + ids[i+1];
     return id;
}

static inline void get_indexes(int id, int *ids, int ndim, int *dims)
{
     for (int i = ndim-1; i >=0; i--) {
          ids[i] = id % dims[i];
          id = id / dims[i];
     }
}

static inline int compute_length(int ndim, const int *dims)
{
     int i, len;

     assert(ndim > 0);
     assert(dims);
     for (i = 0, len = 1; i < ndim; i++) {
          assert(dims[i] > 0);
          len *= dims[i];
     }
     return len;
}

static inline void check_dim(int ndim, const int *dims)
{
     int i;

     assert(ndim > 0);
     assert(dims);
     for (i = 0; i < ndim; i++)
          assert(dims[i] > 0);
}

static inline void check_tensor(const tl_tensor *t)
{
     assert(t && t->data);
     assert(t->dtype >= 0 && t->dtype < TL_DTYPE_SIZE);
     assert(t->len == compute_length(t->ndim, t->dims));
}

int tl_tensor_issameshape(const tl_tensor *t1, const tl_tensor *t2)
{
     int ndim;

     assert(t1 && t2);
     if (t1->ndim == t2->ndim) {
          ndim = t1->ndim;
          while (--ndim >= 0)
               if (t1->dims[ndim] != t2->dims[ndim])
                    return 0;
          return 1;
     }
     return 0;
}

tl_tensor *tl_tensor_create(void *data, int ndim, const int *dims,
                            tl_dtype dtype)
{
     tl_tensor *t;
     size_t size;

     t = (tl_tensor *)tl_alloc(sizeof(tl_tensor));
     t->len = compute_length(ndim, dims);
     t->ndim = ndim;
     t->dims = (int *)tl_clone(dims, sizeof(int) * ndim);
     t->dtype = dtype;
     size = t->len * tl_size_of(dtype);
     if (!data) {
          t->data = tl_alloc(size);
          memset(t->data, 0, size);
     } else {
          t->data = data;
     }

     return t;
}

void tl_tensor_free(tl_tensor *t)
{
     assert(t);
     tl_free(t->dims);
     tl_free(t);
}

void tl_tensor_free_data_too(tl_tensor *t)
{
     tl_free(t->data);
     tl_tensor_free(t);
}

/* TODO: va_list length not checked */
tl_tensor *tl_tensor_zeros(tl_dtype dtype, int ndim, ...)
{
     tl_tensor *t;
     int *dims;
     va_list ap;
     int i;

     assert(ndim > 0);
     dims = (int *)tl_alloc(sizeof(int) * ndim);
     va_start(ap, ndim);
     for (i = 0; i < ndim; i++) {
          dims[i] = va_arg(ap, int);
          assert(dims[i] > 0);
     }
     va_end(ap);

     t = tl_tensor_create(NULL, ndim, dims, dtype);
     tl_free(dims);
     return t;
}

/* same as tl_tensor_zeros(), just for completeness */
tl_tensor *tl_tensor_vcreate(tl_dtype dtype, int ndim, ...)
{
     tl_tensor *t;
     int *dims;
     va_list ap;
     int i;

     assert(ndim > 0);
     dims = (int *)tl_alloc(sizeof(int) * ndim);
     va_start(ap, ndim);
     for (i = 0; i < ndim; i++) {
          dims[i] = va_arg(ap, int);
          assert(dims[i] > 0);
     }
     va_end(ap);

     t = tl_tensor_create(NULL, ndim, dims, dtype);
     tl_free(dims);
     return t;
}

tl_tensor *tl_tensor_clone(const tl_tensor *src)
{
     void *data;
     tl_tensor *dst;

     assert(src);
     data = tl_clone(src->data, src->len*tl_size_of(src->dtype));
     dst = tl_tensor_create(data, src->ndim, src->dims, src->dtype);
     return dst;
}

void tl_tensor_fprint(FILE *stream, const tl_tensor *t, const char *fmt)
{
     int ndim, len, *dims; /* pointer short cut */
     void *data;
     tl_dtype dtype;
     size_t dsize;

     /* dimision size and how deep current chars go */
     int *dim_sizes, *dim_levels;
     /* buffer for brackets */
     char *left_buf, *right_buf;
     char *lp, *rp;
     size_t right_len;
     int i, j, k;

     assert(stream && t);
     ndim = t->ndim;
     len = t->len;
     dims = t->dims;
     data = t->data;
     dtype = t->dtype;
     dsize = tl_size_of(dtype);

     dim_sizes = (int *)tl_alloc(sizeof(int) * ndim);
     dim_levels = (int *)tl_alloc(sizeof(int) * ndim);
     dim_sizes[ndim-1] = dims[ndim-1];
     dim_levels[ndim-1] = 0;
     left_buf = (char *)tl_alloc(sizeof(char) * (ndim+1));
     right_buf = (char *)tl_alloc(sizeof(char) * (ndim+1));
     lp = left_buf;
     rp = right_buf;

     for (i = ndim-2; i >= 0; i--) {
          dim_sizes[i] = dims[i] * dim_sizes[i+1];
          dim_levels[i] = 0;
     }
     for (i = 0; i < len; i++) {
          for (j = 0; j < ndim; j++) {
               if (i % dim_sizes[j] == 0)
                    dim_levels[j]++;
               if (dim_levels[j] == 1) {
                    *lp++ = '[';
                    dim_levels[j]++;
               }
               if (dim_levels[j] == 3) {
                    *rp++ = ']';
                    if (j != 0 && dim_levels[j] > dim_levels[j-1]) {
                         *lp++ = '[';
                         dim_levels[j] = 2;
                    } else
                         dim_levels[j] = 0;
               }
          }
          *lp = *rp = '\0';
          fprintf(stream, "%s", right_buf);
          if (*right_buf != '\0') {
               fprintf(stream, "\n");
               right_len = strlen(right_buf);
               for (k = ndim-right_len; k > 0; k--)
                    fprintf(stream, " ");
          }
          fprintf(stream, "%s", left_buf);
          if (*left_buf == '\0')
               fprintf(stream, " ");
          tl_fprintf(stream, fmt, tl_padd(data, i, dsize), dtype);
          lp = left_buf, rp = right_buf;
     }
     for (j = 0; j < ndim; j++)
          fprintf(stream, "]");
     fprintf(stream, "\n");

     tl_free(dim_sizes);
     tl_free(dim_levels);
     tl_free(left_buf);
     tl_free(right_buf);
}

void tl_tensor_print(const tl_tensor *t, const char *fmt)
{
     tl_tensor_fprint(stdout, t, fmt);
}

int tl_tensor_save(const char *file_name, const tl_tensor *t, const char *fmt)
{
     FILE *fp;

     fp = fopen(file_name, "w");
     if (!fp) {
          tl_err_ret("ERROR: cannot open %s", file_name);
          return -1;
     }
     tl_tensor_fprint(fp, t, fmt);
     fclose(fp);
     return 0;
}

tl_tensor *tl_tensor_create_slice(const tl_tensor *src, int axis, int len,
                                  tl_dtype dtype)
{
     tl_tensor *dst;
     int *dims;

     assert(src);
     assert(axis < src->ndim && axis >= 0);
     assert(len <= src->dims[axis] && len > 0);

     dims = (int *)tl_clone(src->dims, sizeof(int) * src->ndim);
     dims[axis] = len;
     dst = tl_tensor_create(NULL, src->ndim, dims, dtype);
     tl_free(dims);

     return dst;
}

tl_tensor *tl_tensor_slice(const tl_tensor *src, tl_tensor *dst, int axis,
                           int start, int len)
{
     int i;
     int d_vol, s_vol, vol;
     int thread_num;
     int si, di;
     size_t dsize;

     assert(src);
     assert(axis < src->ndim && axis >= 0);
     assert(len <= src->dims[axis] && len > 0);
     assert(start < src->dims[axis] && start >= 0);
     assert(len + start <= src->dims[axis]);
     if (dst) {
          assert(src->dtype == dst->dtype);
          assert(dst->ndim == src->ndim);
#ifndef NDEBUG
          for (i = 0; i < src->ndim; i++)
               assert(i == axis ? dst->dims[i] == len :
                      dst->dims[i] == src->dims[i]);
#endif
     } else {
          dst = tl_tensor_create_slice(src, axis, len, src->dtype);
     }

     for (i = axis+1, vol = 1; i < dst->ndim; i++)
          vol *= dst->dims[i];
     d_vol = vol * dst->dims[axis];
     s_vol = vol * src->dims[axis];
     thread_num = dst->len;

     dsize = tl_size_of(src->dtype);
     for (di = 0; di < thread_num; di++) {
          si = di / d_vol * s_vol + di % d_vol + start * vol;
          tl_passign(dst->data, di, src->data, si, dsize);
     }

     return dst;
}

/* in-place reshape tensor */
tl_tensor *tl_tensor_reshape(const tl_tensor *src, int ndim, const int *dims)
{
     tl_tensor *dst;

     assert(src);
     assert(src->len == compute_length(ndim, dims));
     dst = tl_tensor_create(src->data, ndim, dims, src->dtype);
     return dst;
}

tl_tensor *tl_tensor_vreshape(const tl_tensor *src, int ndim, ...)
{
     tl_tensor *dst;
     int *dims;
     va_list ap;
     int i;

     assert(src);
     assert(ndim > 0);
     dims = (int *)tl_alloc(sizeof(int) * ndim);
     va_start(ap, ndim);
     for (i = 0; i < ndim; i++) {
          dims[i] = va_arg(ap, int);
          assert(dims[i] > 0);
     }
     va_end(ap);
     assert(src->len == compute_length(ndim, dims));
     dst = tl_tensor_create(src->data, ndim, dims, src->dtype);
     tl_free(dims);
     return dst;
}

tl_tensor *tl_tensor_maxreduce(const tl_tensor *src, tl_tensor *dst,
                               tl_tensor *arg, int axis)
{
     /* suppose the shape of src is [N, C, H, W], dim = 1, then thread_num is N x H x W
        reduce_vol is H x W, index_vol is C x H x W */
     int thread_num, reduce_vol, index_vol;
     int i, di, si, maxi;
     int dim_size;
     void *data_s, *data_d, *data_a, *nowp, *maxp;
     size_t dsize;
     tl_dtype dtype;
     tl_cmp_func cmp;

     assert(src);
     assert(axis < src->ndim && axis >= 0);
     if (dst) {
          assert(src->dtype == dst->dtype);
#ifndef NDEBUG
          for (i = 0; i < dst->ndim; i++)
               assert(i == axis ? dst->dims[i] == 1 :
                      dst->dims[i] == src->dims[i]);
#endif
     } else {
          dst = tl_tensor_create_slice(src, axis, 1, src->dtype);
     }
     if (arg) {
          assert(arg->dtype == TL_INT32);
          for (i = 0; i < arg->ndim; i++)
               assert(i == axis ? arg->dims[i] == 1 :
                      arg->dims[i] == src->dims[i]);
     }

     for (i = axis+1, thread_num = 1; i < dst->ndim; i++)
          thread_num *= dst->dims[i];
     reduce_vol = thread_num;
     index_vol = thread_num * src->dims[axis];
     for (i = 0; i < axis; i++)
          thread_num *= dst->dims[i];

     dtype = src->dtype;
     cmp = tl_cmp_getfunc(dtype);
     dsize = tl_size_of(dtype);
     dim_size = src->dims[axis];
     nowp = tl_alloc(dsize);
     maxp = tl_alloc(dsize);
     data_s = src->data;
     data_d = dst->data;
     if (arg)
          data_a = arg->data;
     for (di = 0; di < thread_num; di++) {
          /* src[si] is the first element in this thread to be compared, then
             si = batch_vol * batch + (di - reduce_vol * batch),
             where batch = di / reduce_vol,
             which is the same as the following code: */
          si = (index_vol - reduce_vol) * (di / reduce_vol) + di;
          tl_passign(nowp, 0, data_s, si, dsize);
          tl_passign(maxp, 0, nowp, 0, dsize);
          for (i = 1, maxi = 0; i < dim_size; i++) {
               tl_passign(nowp, 0, data_s, si+i*reduce_vol, dsize);
               if (cmp(nowp, maxp) > 0) {
                    tl_passign(maxp, 0, nowp, 0, dsize);
                    maxi = i;
               }
          }
          tl_passign(data_d, di, maxp, 0, dsize);
          if (arg)
               ((int32_t *)data_a)[di] = maxi;
     }
     tl_free(nowp);
     tl_free(maxp);

     return dst;
}

tl_tensor *tl_tensor_elew(const tl_tensor *src1, const tl_tensor *src2,
                          tl_tensor *dst, tl_elew_op elew_op)
{
     int thread_num;
     int di;
     size_t dsize;
     tl_dtype dtype;
     void *s1_data, *s2_data, *d_data;
     void *elew_res;
     tl_elew_func elew;

     assert(tl_tensor_issameshape(src1, src2));
     assert(src1->dtype == src2->dtype);
     if (dst) {
          assert(tl_tensor_issameshape(src1, dst));
          assert(src1->dtype == dst->dtype);
     } else {
          dst = tl_tensor_create(NULL, src1->ndim, src2->dims, src1->dtype);
     }

     thread_num = dst->len;
     s1_data = src1->data;
     s2_data = src2->data;
     d_data = dst->data;
     dtype = src1->dtype;
     dsize = tl_size_of(dtype);
     elew = tl_elew_getfunc(dtype);
     elew_res = tl_alloc(dsize);
     for (di = 0; di < thread_num; di++) {
          elew(tl_padd(s1_data, di, dsize),
               tl_padd(s2_data, di, dsize), elew_res, elew_op);
          tl_passign(d_data, di, elew_res, 0, dsize);
     }
     tl_free(elew_res);

     return dst;
}

/* (optional) workspace size equals (sizeof(int) * dst->ndim * dst->len), two of them */
tl_tensor *tl_tensor_transpose(const tl_tensor *src, tl_tensor *dst,
                               const int *axes, int **workspace)
{
     int *s_ids, *d_ids, *s_dims, *d_dims;
     int thread_num;
     int di, si;
     int ndim;
     int *t_s_ids;
     int *t_d_ids;
     size_t dsize;
     void *s_data, *d_data;
     int i;

     assert(src);
     if (dst) {
          assert(src->dtype == dst->dtype);
          assert(src->len == dst->len);
          assert(src->ndim == dst->ndim);
#ifndef NDEBUG
          for (i = 0; i < dst->ndim; i++)
               assert(src->dims[axes[i]] = dst->dims[i]);
#endif
     } else {
          d_dims = (int *)tl_alloc(src->ndim * sizeof(int));
          for (i = 0; i < src->ndim; i++)
               d_dims[i] = src->dims[axes[i]];
          dst = tl_tensor_create(NULL, src->ndim, d_dims, src->dtype);
          tl_free(d_dims);
     }

     thread_num = dst->len;
     s_dims = (int *)tl_clone(src->dims, sizeof(int) * src->ndim);
     d_dims = (int *)tl_clone(dst->dims, sizeof(int) * dst->ndim);
     if (!workspace) {
          s_ids = (int *)tl_alloc(sizeof(int) * dst->ndim * thread_num);
          d_ids = (int *)tl_alloc(sizeof(int) * dst->ndim * thread_num);
     } else {
          s_ids = workspace[0];
          d_ids = workspace[1];
     }

     dsize = tl_size_of(src->dtype);
     s_data = src->data;
     d_data = dst->data;
     ndim = dst->ndim;
     for (di = 0; di < thread_num; di++) {
          t_s_ids = s_ids + di * ndim;
          t_d_ids = d_ids + di * ndim;
          get_indexes(di, t_d_ids, ndim, d_dims);
          for (i = 0; i < ndim; i++)
               t_s_ids[axes[i]] = t_d_ids[i];
          si = get_index(t_s_ids, ndim, s_dims);

          tl_passign(d_data, di, s_data, si, dsize);
     }

     if (!workspace) {
          tl_free(s_ids);
          tl_free(d_ids);
     }
     tl_free(s_dims);
     tl_free(d_dims);

     return dst;
}

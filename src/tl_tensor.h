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

#ifndef _TL_TENSOR_H_
#define _TL_TENSOR_H_

#include "tl_type.h"

typedef struct tl_tensor tl_tensor;
struct tl_tensor {
     tl_dtype  dtype;
     int       len;
     int       ndim;
     int      *dims;
     void     *data;
};

#ifdef __cplusplus
extern "C" {
#endif

int tl_tensor_issameshape(const tl_tensor *t1, const tl_tensor *t2);
tl_tensor *tl_tensor_create(void *data, int ndim, const int *dims,
                         tl_dtype dtype);
void tl_tensor_free(tl_tensor *t);
void tl_tensor_free_data_too(tl_tensor *t);
tl_tensor *tl_tensor_zeros(tl_dtype dtype, int ndim, ...);
tl_tensor *tl_tensor_vcreate(tl_dtype dtype, int ndim, ...);
tl_tensor *tl_tensor_clone(const tl_tensor *src);
void tl_tensor_fprint(FILE *stream, const tl_tensor *t, const char *fmt);
void tl_tensor_print(const tl_tensor *t, const char *fmt);
int tl_tensor_save(const char *file_name, const tl_tensor *t, const char *fmt);
tl_tensor *tl_tensor_create_slice(const tl_tensor *src, int axis, int len,
                              tl_dtype dtype);
tl_tensor *tl_tensor_slice(const tl_tensor *src, tl_tensor *dst, int axis,
                         int start, int len);
tl_tensor *tl_tensor_reshape(const tl_tensor *src, int ndim, const int *dims);
tl_tensor *tl_tensor_vreshape(const tl_tensor *src, int ndim, ...);
tl_tensor *tl_tensor_maxreduce(const tl_tensor *src, tl_tensor *dst,
                              tl_tensor *arg, int axis);
tl_tensor *tl_tensor_elew(const tl_tensor *src1, const tl_tensor *src2,
                         tl_tensor *dst, tl_elew_op elew_op);
tl_tensor *tl_tensor_transpose(const tl_tensor *src, tl_tensor *dst,
                              const int *axes, int **workspace);

#ifdef __cplusplus
}
#endif

#endif  /* _TL_TENSOR_H_ */

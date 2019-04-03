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

#ifndef _LN_CUDA_H_
#define _LN_CUDA_H_
#ifdef LN_CUDA

#include "ln_util.h"

typedef struct ln_cuda_stream ln_cuda_stream;

#define LN_MAX_CUDA_DEVICE 15

#ifdef __cplusplus
LN_CPPSTART
#endif

void ln_cuda_set_device(int n);
int ln_cuda_get_device();
int ln_is_device_mem(const void *ptr);
void *ln_alloc_cuda(size_t size);
void *ln_memset_cuda(void *dst, int c, size_t n);
void *ln_memcpy_h2d(void *dst, const void *src, size_t size);
void *ln_memcpy_d2h(void *dst, const void *src, size_t size);
void *ln_memcpy_d2d(void *dst, const void *src, size_t size);
void ln_free_cuda(void *p);
void *ln_clone_h2d(const void *src, size_t size);
void *ln_clone_d2h(const void *src, size_t size);
void *ln_clone_d2d(const void *src, size_t size);
void *ln_repeat_h2d(void *data, size_t size, int times);
void *ln_repeat_d2h(void *data, size_t size, int times);
void *ln_repeat_d2d(void *data, size_t size, int times);
void ln_cuda_stream_create(ln_cuda_stream *pstream);
void ln_cuda_stream_sync(ln_cuda_stream stream);
void ln_cuda_device_reset(void);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif  /* LN_CUDA */
#endif  /* _LN_CUDA_H_ */

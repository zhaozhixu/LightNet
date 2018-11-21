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

#ifndef _LN_UTIL_H_
#define _LN_UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

enum ln_bool {
    LN_FALSE = 0,
    LN_TRUE = 1
};
typedef enum ln_bool ln_bool;

typedef int (*ln_cmp_func)(void *, void *);
typedef void (*ln_free_func)(void *);
typedef void (*ln_fprint_func) (FILE *, void *);

#define ln_free free

#define LN_MAXLINE 4096

#ifdef __cplusplus
#define LN_CPPSTART extern "C" {
#define LN_CPPEND }
LN_CPPSTART
#endif

void *ln_alloc(size_t size);
void *ln_realloc(void *ptr, size_t size);
char *ln_strdup(const char *s);
char *ln_path_alloc(size_t *sizep);
void *ln_clone(const void *src, size_t size);
void *ln_repeat(void *data, size_t size, int times);
char *ln_read_text(const char *file_name);
char *ln_next_token(const char *s, int c);
char *ln_strcat_alloc(const char *s1, const char *s2);
char *ln_strcat_delim_alloc(const char *s1, const char *s2, char delim);
int ln_streq(const char *s1, const char *s2);
int ln_compute_output_dim(int input_dim, int size, int stride, int padding);
int ln_compute_length(int ndim, const int *dims);
void ln_err_msg(const char *fmt, ...);
void ln_err_cont(int error, const char *fmt, ...);
void ln_err_ret(const char *fmt, ...);
void ln_err_quit(const char *fmt, ...);
void ln_err_bt(const char *fmt, ...);
void ln_err_exit(int error, const char *fmt, ...);
void ln_err_sys(const char *fmt, ...);
void ln_err_dump(const char *fmt, ...);

#ifdef LN_CUDA

typedef struct ln_cuda_stream ln_cuda_stream;

#define LN_MAX_CUDA_DEVICE 15

void ln_cuda_set_device(int n);
int ln_cuda_get_device();
int ln_is_device_mem(const void *ptr);
void *ln_alloc_cuda(size_t size);
void ln_memset_cuda(void *dst, int c, size_t n);
void ln_memcpy_h2d(void *dst, const void *src, size_t size);
void ln_memcpy_d2h(void *dst, const void *src, size_t size);
void ln_memcpy_d2d(void *dst, const void *src, size_t size);
void ln_free_cuda(void *p);
void *ln_clone_h2d(const void *src, size_t size);
void *ln_clone_d2h(const void *src, size_t size);
void *ln_clone_d2d(const void *src, size_t size);
void *ln_repeat_h2d(void *data, size_t size, int times);
void *ln_repeat_d2h(void *data, size_t size, int times);
void *ln_repeat_d2d(void *data, size_t size, int times);
void ln_cuda_stream_create(ln_cuda_stream *pstream);
void ln_cuda_stream_sync(ln_cuda_stream stream);

#endif  /* LN_CUDA */

#ifdef __cplusplus
LN_CPPEND
#endif

#endif	/* _LN_UTIL_H_ */

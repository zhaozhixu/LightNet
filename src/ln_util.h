/*
 * Copyright (c) 2018-2020 Zhixu Zhao
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
#include <string.h>
#include <time.h>

#include "tl_tensor.h"
#include "ln_util_common.h"

enum ln_bool {
    LN_FALSE = 0,
    LN_TRUE = 1
};
typedef enum ln_bool ln_bool;

typedef int (*ln_cmp_func)(const void *p1, const void *p2);
typedef void (*ln_free_func)(void *p);
typedef void (*ln_fprint_func) (FILE *fp, const void *p);
typedef uint32_t (*ln_hash_func)(const void *p);
typedef void *(*ln_copy_func)(void *dst, const void *src, size_t n);

#define LN_EXPORT __attribute__ ((visibility ("default")))

#define LN_MAXLINE 4096

#define LN_MAX_NAME_LEN 512      /* including the terminating null byte */
#define LN_MAX_NAME_IDX (UINT32_MAX)
#define LN_MAX_NAME_IDX_LEN 10  /* 2^32 =  4294967296, 10 chars*/
#define LN_MAX_NAME_SUFFIX (LN_MAX_NAME_IDX_LEN)
#define LN_MAX_NAME_PREFIX (LN_MAX_NAME_LEN - LN_MAX_NAME_SUFFIX - 1)

#ifdef __cplusplus
LN_CPPSTART
#endif

char **ln_strarraydup(char *const *sa, int len);
void ln_strarray_free(char **sa, int len);
char *ln_path_alloc(size_t *sizep);
void *ln_clone(const void *src, size_t size);
void *ln_repeat(void *data, size_t size, int times);
char *ln_read_text(const char *path);
char *ln_read_stdin(void);
char *ln_next_token(const char *s, int c);
char *ln_strcat_alloc(const char *s1, const char *s2);
char *ln_strcat_delim_alloc(const char *s1, const char *s2, char delim);
int ln_is_prefix_plus_digit(const char *str, const char *prefix);
int ln_digit_num(ssize_t num);
int ln_output_dim_conv(int input_dim, int size, int stride, int padding,
                       int dilation);
int ln_output_dim_deconv(int input_dim, int size, int stride, int padding,
                         int output_padding, int dilation);
int *ln_autopadding_conv(int *padding, const int *input_dims, const int *size,
                         const int *stride, const int *dilations, int ndim,
                         const char *mode);
int *ln_autopadding_deconv(int *padding, const int *input_dims,
                           const int *output_dims, const int *size,
                           const int *stride, const int *output_padding,
                           const int *dilations, int ndim, const char *mode);
int ln_compute_length(int ndim, const int *dims);
void ln_print_shape(int ndim, int *dims);
char *ln_sprint_shape(char *buf, int ndim, int *dims);
uint32_t ln_direct_hash(const void *key);
int ln_direct_cmp(const void *p1, const void *p2);
uint32_t ln_str_hash(const void *key);
int ln_str_cmp(const void *p1, const void *p2);
void ln_img_submean(const unsigned char *data, const float *mean, float *out,
                    int H, int W, int C);
int ln_next_multiple_power2(int n, int power2);
void ln_err_msg(const char *fmt, ...);
void ln_err_cont(int error, const char *fmt, ...);
void ln_err_ret(const char *fmt, ...);
void ln_err_quit(const char *fmt, ...);
void ln_err_bt(const char *fmt, ...);
void ln_err_exit(int error, const char *fmt, ...);
void ln_err_sys(const char *fmt, ...);
void ln_err_dump(const char *fmt, ...);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif	/* _LN_UTIL_H_ */

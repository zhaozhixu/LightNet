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

typedef enum ln_bool ln_bool;
enum ln_bool {
     LN_FALSE = 0,
     LN_TRUE = 1
};

typedef int (*ln_cmp_func)(void *, void *);

#define ln_free free

#define LN_MAXLINE 4096

#ifdef __cplusplus
#define LN_CPPSTART extern "C" {
#define LN_CPPEND }
LN_CPPSTART
#endif

void *ln_alloc(size_t size);
char *ln_path_alloc(size_t *sizep);
void *ln_clone(const void *src, size_t size);
void *ln_repeat(void *data, size_t size, int times);
void ln_err_msg(const char *fmt, ...);
void ln_err_cont(int error, const char *fmt, ...);
void ln_err_ret(const char *fmt, ...);
void ln_err_quit(const char *fmt, ...);
void ln_err_exit(int error, const char *fmt, ...);
void ln_err_sys(const char *fmt, ...);
void ln_err_dump(const char *fmt, ...);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif	/* _LN_UTIL_H_ */

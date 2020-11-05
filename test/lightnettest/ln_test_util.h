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

#ifndef _LN_TEST_UTIL_H_
#define _LN_TEST_UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define ln_test_free free

#define ln_test_free_arrays(arr, len)           \
    do {                                        \
        for (int _i = 0; _i < (len); _i++)      \
            free((arr)[_i]);                    \
        free((arr));                            \
    } while(0)

#ifdef __cplusplus
#define LN_TEST_CPPSTART extern "C" {
#define LN_TEST_CPPEND }
LN_TEST_CPPSTART
#endif

void *ln_test_alloc(size_t size);
void *ln_test_realloc(void *ptr, size_t size);
char *ln_test_strdup(const char *s);

#ifdef __cplusplus
LN_TEST_CPPEND
#endif

#endif  /* _LN_TEST_UTIL_H_ */

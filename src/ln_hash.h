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

#ifndef _LN_HASH_H_
#define _LN_HASH_H_

#include "ln_util.h"

typedef struct ln_hash ln_hash;
typedef int (*ln_hash_func)(void *key);

#ifdef __cplusplus
LN_CPPSTART
#endif

ln_hash *ln_hash_create(ln_hash_func hash_func, ln_cmp_func cmp_func,
                        ln_free_func free_k_func, ln_free_func free_v_func);
void ln_hash_free(ln_hash *hash);
int ln_hash_insert(ln_hash *hash, void *key, void *value);
void *ln_hash_find(ln_hash *hash, void *key);
int ln_hash_find_extended(ln_hash *hash, void *key, void **value);
int ln_hash_remove(ln_hash *hash, void *key);
int ln_hash_size(ln_hash *hash);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif  /* _LN_HASH_H_ */

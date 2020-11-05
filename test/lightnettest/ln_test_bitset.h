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

#ifndef _LN_TEST_BITSET_H_
#define _LN_TEST_BITSET_H_

#include <stdint.h>
#include "ln_test_util.h"

struct ln_test_bitset {
    uint64_t *ints;
    int       num_ints;
    int       num_bits;
};
typedef struct ln_test_bitset ln_test_bitset;

#ifdef __cplusplus
LN_TEST_CPPSTART
#endif

ln_test_bitset *ln_test_bitset_create(int init_num_bits);
void ln_test_bitset_free(ln_test_bitset *bitset);
ln_test_bitset *ln_test_bitset_ones(int num_ones);
void ln_test_bitset_zero(ln_test_bitset *bitset);
ln_test_bitset *ln_test_bitset_copy(ln_test_bitset *bitset);
int ln_test_bitset_population(const ln_test_bitset *bitset);
int *ln_test_bitset_create_one_indexes(const ln_test_bitset *bitset, int *count);
void ln_test_bitset_expand(ln_test_bitset *bitset, int num_bits);
void ln_test_bitset_set(ln_test_bitset *bitset, int n);
void ln_test_bitset_clear(ln_test_bitset *bitset, int n);
void ln_test_bitset_and_assign(ln_test_bitset *bitset1, ln_test_bitset *bitset2);
void ln_test_bitset_or_assign(ln_test_bitset *bitset1, ln_test_bitset *bitset2);
void ln_test_bitset_not_assign(ln_test_bitset *bitset);

#ifdef __cplusplus
LN_TEST_CPPEND
#endif

#endif  /* _LN_TEST_BITSET_H_ */

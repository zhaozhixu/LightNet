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

#ifndef _LN_NAME_H_
#define _LN_NAME_H_

#include "ln_hash.h"

#define LN_MAX_NAME_LEN 512      /* including the terminating null byte ('\0') */
#define LN_MAX_NAME_IDX (UINT32_MAX)
#define LN_MAX_NAME_IDX_LEN 10  /* 2^32 =  4294967296, 10 chars*/
#define LN_MAX_NAME_SUBFIX (LN_MAX_NAME_IDX_LEN)
#define LN_MAX_NAME_PREFIX (LN_MAX_NAME_LEN - LN_MAX_NAME_SUBFIX - 1)

#ifdef __cplusplus
LN_CPPSTART
#endif

void ln_name_init(void);
void ln_name_cleanup(void);
/* copy before using */
const char *ln_name_unique(const char *prefix);
void ln_name_set_idx(const char *prefix, uint32_t idx);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif  /* _LN_NAME_H_ */

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

#ifndef _LN_TENSORRT_H_
#define _LN_TENSORRT_H_
#ifdef LN_TENSORRT

#include "ln_util.h"

struct ln_tensorrt_bundle;
typedef struct ln_tensorrt_bundle ln_tensorrt_bundle;

#ifdef __cplusplus
LN_CPPSTART
#endif

const char *ln_tensorrt_version_str(void);
void ln_tensorrt_check_op(ln_op_arg *op_arg, ln_msg **error);
ln_tensorrt_bundle *ln_tensorrt_bundle_create(ln_op_arg *op_arg);
void ln_tensorrt_bundle_free(ln_tensorrt_bundle *bundle);
void ln_tensorrt_bundle_execute(ln_tensorrt_bundle *bundle);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif  /* LN_TENSORRT */
#endif  /* _LN_TENSORRT_H_ */

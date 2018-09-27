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

#ifndef _TEST_LIGHTNET_H_
#define _TEST_LIGHTNET_H_

#include <stdio.h>
#include <check.h>
#include <math.h>
#include <tl_check.h>

#ifdef __cplusplus
#define CPPSTART extern "C" {
#define CPPEND }
#endif

Suite *make_master_suite(void);
Suite *make_list_suite(void);
Suite *make_hash_suite(void);
Suite *make_error_suite(void);
Suite *make_param_suite(void);
Suite *make_tensor_suite(void);
Suite *make_op_suite(void);
Suite *make_parse_suite(void);
Suite *make_mem_suite(void);
Suite *make_pass_suite(void);
/* end of declarations */

#ifdef __cplusplus
CPPEND
#endif

#endif /* _TEST_LIGHTNET_H_ */

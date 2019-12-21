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

#ifndef _TEST_LIGHTNET_H_
#define _TEST_LIGHTNET_H_

#include <check.h>
#include <stdio.h>
#include <math.h>

#include "test_record.h"

#ifndef LN_TEST_RESULT_DIR
#define LN_TEST_RESULT_DIR "."
#endif

#ifndef LN_TEST_DATA_DIR
#define LN_TEST_DATA_DIR "."
#endif

#ifdef __cplusplus
TEST_CPPSTART
#endif

Suite *make_master_suite(void);
void add_util_record(test_record *record);
#ifdef LN_CUDA
void add_cuda_record(test_record *record);
#endif
void add_list_record(test_record *record);
void add_queue_record(test_record *record);
void add_stack_record(test_record *record);
void add_hash_record(test_record *record);
void add_graph_record(test_record *record);
void add_msg_record(test_record *record);
void add_param_record(test_record *record);
void add_tensor_record(test_record *record);
void add_op_record(test_record *record);
void add_parse_record(test_record *record);
void add_mem_record(test_record *record);
void add_pass_record(test_record *record);
void add_opimpl_record(test_record *record);
/* end of declarations */

#ifdef __cplusplus
TEST_CPPEND
#endif

#endif /* _TEST_LIGHTNET_H_ */

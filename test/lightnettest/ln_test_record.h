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

#ifndef _LN_TEST_RECORD_H_
#define _LN_TEST_RECORD_H_

#include "ln_test_util.h"

struct ln_test_record {
    const char ***test_names_array;
    int          *test_nums;
    const char  **suite_names;
    const void  **suite_ptrs;
    int           suites_num;
};
typedef struct ln_test_record ln_test_record;

#ifdef __cplusplus
LN_TEST_CPPSTART
#endif

void ln_test_record_init(ln_test_record *record);
ln_test_record *ln_test_record_create(void);
void ln_test_record_finalize(ln_test_record *record);
void ln_test_record_free(ln_test_record *record);
int ln_test_record_add_suite(ln_test_record *record, const char *suite_name,
                             const void *suite_ptr);
int ln_test_record_add_test(ln_test_record *record, const char *suite_name,
                            const char *test_name);
const void *ln_test_record_find_suite_ptr(const ln_test_record *record,
                                          const char *name);
ln_test_record *ln_test_record_create_filtered(const ln_test_record *record,
                                               const char *glob);
void ln_test_record_print(const ln_test_record *record);

#ifdef __cplusplus
LN_TEST_CPPEND
#endif

#endif  /* _LN_TEST_RECORD_H_ */

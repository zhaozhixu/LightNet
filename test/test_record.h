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

#ifndef _TEST_RECORD_H_
#define _TEST_RECORD_H_

struct test_record {
    const char ***tcase_names_array;
    const void ***tcase_ptrs_array;
    int          *tcase_nums;
    const char  **suite_names;
    int           suites_num;
};
typedef struct test_record test_record;

#ifdef __cplusplus
#define TEST_CPPSTART extern "C" {
#define TEST_CPPEND }
#endif

test_record *test_record_create(void);
test_record *test_record_create_filtered(const test_record *record,
                                         const char *glob);
void test_record_free(test_record *record);
int test_record_add_suite(test_record *record, const char *suite_name);
int test_record_add_tcase(test_record *record, const char *suite_name,
                          const char *tcase_name, const void *tcase_ptr);

#ifdef __cplusplus
TEST_CPPEND
#endif

#endif  /* _TEST_RECORD_H_ */

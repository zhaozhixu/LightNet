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

#ifndef _LN_TEST_H_
#define _LN_TEST_H_

#include <check.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "ln_test_record.h"

#ifndef LN_TEST_DIR
#error LN_TEST_DIR should be defined in the command line
#endif

#ifndef LN_BUILD_TEST_DIR
#error LN_BUILD_TEST_DIR should be defined in the command line
#endif

struct ln_test_info {
    int            verbose;
    ln_test_record record;
};
typedef struct ln_test_info ln_test_info;

#define LN_TEST_START(test_name)                                \
    extern ln_test_info _global_test_info;                      \
    START_TEST(test_name) {                                     \
    if (_global_test_info.verbose) printf("\t%s\n", #test_name);

#define LN_TEST_END } END_TEST

#define LN_TEST_TCASE_START_TIMEOUT(tcase_name, setup, teardown, timeout) \
    static TCase *_make_##tcase_name##_tcase(                             \
        ln_test_record *_record, const char *const *_candidates, int _num) { \
    const char *_tcase_name = #tcase_name;                              \
    TCase *_tc = NULL;                                                  \
    if (!_record) {                                                     \
        _tc = tcase_create(#tcase_name);                                \
        tcase_add_checked_fixture(_tc, setup, teardown);                \
        if ((timeout) >= 0)                                             \
            tcase_set_timeout(_tc, (timeout));                          \
    }

#define LN_TEST_TCASE_START(tcase_name, setup, teardown)            \
    LN_TEST_TCASE_START_TIMEOUT(tcase_name, setup, teardown, -1)

#define LN_TEST_ADD_TEST(test_name)                                     \
    do {                                                                \
        if (_record) {                                                  \
            ln_test_record_add_test(_record, _tcase_name, #test_name);  \
            break;                                                      \
        }                                                               \
        if (_num < 0) {                                                 \
            tcase_add_test(_tc, test_name);                             \
            break;                                                      \
        }                                                               \
        for (int _i = 0; _i < _num; _i++) {                             \
            if (!strcmp(#test_name, _candidates[_i])) {                 \
                tcase_add_test(_tc, test_name);                         \
                break;                                                  \
            }                                                           \
        }                                                               \
    } while(0)

#define LN_TEST_TCASE_END return _tc; }

#define LN_TEST_ADD_TCASE(tcase_name)                                   \
    extern ln_test_info _global_test_info;                              \
    static void __attribute__ ((constructor)) _add_##tcase_name##_record(void) \
    {                                                                   \
        ln_test_record_add_suite(&_global_test_info.record, #tcase_name, \
                                 _make_##tcase_name##_tcase);           \
        _make_##tcase_name##_tcase(&_global_test_info.record, NULL, 0); \
    }

#ifdef __cplusplus
LN_TEST_CPPSTART
#endif

void ln_test_set_verbose(int verbose);
int ln_test_run_tests(const char *filter);

#ifdef __cplusplus
LN_TEST_CPPEND
#endif

#endif /* _LN_TEST_H_ */

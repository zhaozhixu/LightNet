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

#include "test_lightnet.h"
#include "../src/ln_util.h"

static void checked_setup(void)
{
}

static void checked_teardown(void)
{
}

START_TEST(test_ln_strcat_delim_alloc)
{
     const char *str1 = "hello";
     const char *str2 = "world";
     char *str = ln_strcat_delim_alloc(str1, str2, ',');
     ck_assert_str_eq(str, "hello,world");
     ln_free(str);
}
END_TEST

START_TEST(test_ln_next_multiple_power2)
{
     ck_assert_int_eq(8, ln_next_multiple_power2(7, 8));
     ck_assert_int_eq(40, ln_next_multiple_power2(39, 8));
     ck_assert_int_eq(-40, ln_next_multiple_power2(-41, 8));
     ck_assert_int_eq(0, ln_next_multiple_power2(-2, 8));
     ck_assert_int_eq(-8, ln_next_multiple_power2(-8, 8));
     ck_assert_int_eq(0, ln_next_multiple_power2(0, 8));
     ck_assert_int_eq(8, ln_next_multiple_power2(8, 8));
     ck_assert_int_eq(8, ln_next_multiple_power2(2, 8));
}
END_TEST

START_TEST(test_ln_autopadding_conv)
{
     int padding[4];
     int *size;
     int *stride;
     int *dims;
     int ndim = 2;

     size = ck_array(int, 1, 1);
     stride = ck_array(int, 1, 1);
     dims = ck_array(int, 4, 4);
     ln_autopadding_conv(padding, dims, size, stride, ndim, "VALID");
     ck_assert_array_int_eq(padding, ck_array(int, 0, 0, 0, 0), 4);
     ln_autopadding_conv(padding, dims, size, stride, ndim, "SAME_UPPER");
     ck_assert_array_int_eq(padding, ck_array(int, 0, 0, 0, 0), 4);
     ln_autopadding_conv(padding, dims, size, stride, ndim, "SAME_LOWER");
     ck_assert_array_int_eq(padding, ck_array(int, 0, 0, 0, 0), 4);

     size = ck_array(int, 2, 2);
     stride = ck_array(int, 1, 1);
     dims = ck_array(int, 4, 4);
     ln_autopadding_conv(padding, dims, size, stride, ndim, "VALID");
     ck_assert_array_int_eq(padding, ck_array(int, 0, 0, 0, 0), 4);
     ln_autopadding_conv(padding, dims, size, stride, ndim, "SAME_UPPER");
     ck_assert_array_int_eq(padding, ck_array(int, 0, 0, 1, 1), 4);
     ln_autopadding_conv(padding, dims, size, stride, ndim, "SAME_LOWER");
     ck_assert_array_int_eq(padding, ck_array(int, 1, 1, 0, 0), 4);

     size = ck_array(int, 3, 3);
     stride = ck_array(int, 1, 1);
     dims = ck_array(int, 8, 8);
     ln_autopadding_conv(padding, dims, size, stride, ndim, "VALID");
     ck_assert_array_int_eq(padding, ck_array(int, 0, 0, 0, 0), 4);
     ln_autopadding_conv(padding, dims, size, stride, ndim, "SAME_UPPER");
     ck_assert_array_int_eq(padding, ck_array(int, 1, 1, 1, 1), 4);
     ln_autopadding_conv(padding, dims, size, stride, ndim, "SAME_LOWER");
     ck_assert_array_int_eq(padding, ck_array(int, 1, 1, 1, 1), 4);

     size = ck_array(int, 3, 3);
     stride = ck_array(int, 2, 2);
     dims = ck_array(int, 8, 8);
     ln_autopadding_conv(padding, dims, size, stride, ndim, "VALID");
     ck_assert_array_int_eq(padding, ck_array(int, 0, 0, 0, 0), 4);
     ln_autopadding_conv(padding, dims, size, stride, ndim, "SAME_UPPER");
     ck_assert_array_int_eq(padding, ck_array(int, 0, 0, 1, 1), 4);
     ln_autopadding_conv(padding, dims, size, stride, ndim, "SAME_LOWER");
     ck_assert_array_int_eq(padding, ck_array(int, 1, 1, 0, 0), 4);

     size = ck_array(int, 3, 3);
     stride = ck_array(int, 2, 2);
     dims = ck_array(int, 9, 9);
     ln_autopadding_conv(padding, dims, size, stride, ndim, "VALID");
     ck_assert_array_int_eq(padding, ck_array(int, 0, 0, 0, 0), 4);
     ln_autopadding_conv(padding, dims, size, stride, ndim, "SAME_UPPER");
     ck_assert_array_int_eq(padding, ck_array(int, 1, 1, 1, 1), 4);
     ln_autopadding_conv(padding, dims, size, stride, ndim, "SAME_LOWER");
     ck_assert_array_int_eq(padding, ck_array(int, 1, 1, 1, 1), 4);
}
END_TEST
/* end of tests */

Suite *make_util_suite(void)
{
     Suite *s;
     TCase *tc_util;

     s = suite_create("util");
     tc_util = tcase_create("util");
     tcase_add_checked_fixture(tc_util, checked_setup, checked_teardown);

     tcase_add_test(tc_util, test_ln_strcat_delim_alloc);
     tcase_add_test(tc_util, test_ln_next_multiple_power2);
     tcase_add_test(tc_util, test_ln_autopadding_conv);
     /* end of adding tests */

     suite_add_tcase(s, tc_util);

     return s;
}

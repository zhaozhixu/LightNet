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

#include <check.h>
#include <tl_check.h>
#include "lightnettest/ln_test.h"
#include "../src/ln_util.h"

static void checked_setup(void)
{
}

static void checked_teardown(void)
{
}

LN_TEST_START(test_ln_strcat_delim_alloc)
{
    const char *str1 = "hello";
    const char *str2 = "world";
    char *str = ln_strcat_delim_alloc(str1, str2, ',');
    ck_assert_str_eq(str, "hello,world");
    ln_free(str);
}
LN_TEST_END

LN_TEST_START(test_ln_next_multiple_power2)
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
LN_TEST_END

LN_TEST_START(test_ln_autopadding_conv)
{
    int padding[4];
    int *size;
    int *stride;
    int *dims;
    int ndim = 2;

    size = ck_array(int, 1, 1);
    stride = ck_array(int, 1, 1);
    dims = ck_array(int, 4, 4);
    ln_autopadding_conv(padding, dims, size, stride, (int[]){1, 1}, ndim, "VALID");
    ck_assert_array_int_eq(padding, ck_array(int, 0, 0, 0, 0), 4);
    ln_autopadding_conv(padding, dims, size, stride, (int[]){1, 1}, ndim, "SAME_UPPER");
    ck_assert_array_int_eq(padding, ck_array(int, 0, 0, 0, 0), 4);
    ln_autopadding_conv(padding, dims, size, stride, (int[]){1, 1}, ndim, "SAME_LOWER");
    ck_assert_array_int_eq(padding, ck_array(int, 0, 0, 0, 0), 4);

    size = ck_array(int, 2, 2);
    stride = ck_array(int, 1, 1);
    dims = ck_array(int, 4, 4);
    ln_autopadding_conv(padding, dims, size, stride, (int[]){1, 1}, ndim, "VALID");
    ck_assert_array_int_eq(padding, ck_array(int, 0, 0, 0, 0), 4);
    ln_autopadding_conv(padding, dims, size, stride, (int[]){1, 1}, ndim, "SAME_UPPER");
    ck_assert_array_int_eq(padding, ck_array(int, 0, 0, 1, 1), 4);
    ln_autopadding_conv(padding, dims, size, stride, (int[]){1, 1}, ndim, "SAME_LOWER");
    ck_assert_array_int_eq(padding, ck_array(int, 1, 1, 0, 0), 4);

    size = ck_array(int, 3, 3);
    stride = ck_array(int, 1, 1);
    dims = ck_array(int, 8, 8);
    ln_autopadding_conv(padding, dims, size, stride, (int[]){1, 1}, ndim, "VALID");
    ck_assert_array_int_eq(padding, ck_array(int, 0, 0, 0, 0), 4);
    ln_autopadding_conv(padding, dims, size, stride, (int[]){1, 1}, ndim, "SAME_UPPER");
    ck_assert_array_int_eq(padding, ck_array(int, 1, 1, 1, 1), 4);
    ln_autopadding_conv(padding, dims, size, stride, (int[]){1, 1}, ndim, "SAME_LOWER");
    ck_assert_array_int_eq(padding, ck_array(int, 1, 1, 1, 1), 4);

    size = ck_array(int, 3, 3);
    stride = ck_array(int, 2, 2);
    dims = ck_array(int, 8, 8);
    ln_autopadding_conv(padding, dims, size, stride, (int[]){1, 1}, ndim, "VALID");
    ck_assert_array_int_eq(padding, ck_array(int, 0, 0, 0, 0), 4);
    ln_autopadding_conv(padding, dims, size, stride, (int[]){1, 1}, ndim, "SAME_UPPER");
    ck_assert_array_int_eq(padding, ck_array(int, 0, 0, 1, 1), 4);
    ln_autopadding_conv(padding, dims, size, stride, (int[]){1, 1}, ndim, "SAME_LOWER");
    ck_assert_array_int_eq(padding, ck_array(int, 1, 1, 0, 0), 4);

    size = ck_array(int, 3, 3);
    stride = ck_array(int, 2, 2);
    dims = ck_array(int, 9, 9);
    ln_autopadding_conv(padding, dims, size, stride, (int[]){1, 1}, ndim, "VALID");
    ck_assert_array_int_eq(padding, ck_array(int, 0, 0, 0, 0), 4);
    ln_autopadding_conv(padding, dims, size, stride, (int[]){1, 1}, ndim, "SAME_UPPER");
    ck_assert_array_int_eq(padding, ck_array(int, 1, 1, 1, 1), 4);
    ln_autopadding_conv(padding, dims, size, stride, (int[]){1, 1}, ndim, "SAME_LOWER");
    ck_assert_array_int_eq(padding, ck_array(int, 1, 1, 1, 1), 4);
}
LN_TEST_END

LN_TEST_START(test_ln_autopadding_deconv)
{
    /* int padding[4]; */
    /* int *size; */
    /* int *stride; */
    /* int *in_dims; */
    /* int *out_dims; */
    /* int ndim = 2; */


}
LN_TEST_END
/* end of tests */

LN_TEST_START(test_ln_subfixed)
{
    ck_assert_int_eq(ln_subfixed("aaabbb", "bbb"), 1);
    ck_assert_int_eq(ln_subfixed("bbb", "bbb"), 1);
    ck_assert_int_eq(ln_subfixed("aaabbb", ""), 1);
    ck_assert_int_eq(ln_subfixed("", ""), 1);
    ck_assert_int_eq(ln_subfixed("aaabbb", "bbbb"), 0);
    ck_assert_int_eq(ln_subfixed("abbb", "bbbb"), 0);
    ck_assert_int_eq(ln_subfixed("bbb", "bbbb"), 0);
    ck_assert_int_eq(ln_subfixed("", "bbbb"), 0);
    ck_assert_int_eq(ln_subfixed(NULL, ""), 0);
    ck_assert_int_eq(ln_subfixed("", NULL), 0);
}
LN_TEST_END

LN_TEST_START(test_ln_is_prefix_plus_digit)
{
    ck_assert_int_eq(ln_is_prefix_plus_digit("aaa1", "aaa"), 1);
    ck_assert_int_eq(ln_is_prefix_plus_digit("aaa12", "aaa"), 1);
    ck_assert_int_eq(ln_is_prefix_plus_digit("aaa12", "aaa1"), 1);
    ck_assert_int_eq(ln_is_prefix_plus_digit("1", ""), 1);
    ck_assert_int_eq(ln_is_prefix_plus_digit("aaa1", "aa"), 0);
    ck_assert_int_eq(ln_is_prefix_plus_digit("aaa", "aaa"), 0);
    ck_assert_int_eq(ln_is_prefix_plus_digit("aaa12", "aa12"), 0);
    ck_assert_int_eq(ln_is_prefix_plus_digit(NULL, ""), 0);
    ck_assert_int_eq(ln_is_prefix_plus_digit("", NULL), 0);
}
LN_TEST_END

LN_TEST_TCASE_START(util, checked_setup, checked_teardown)
{
    LN_TEST_ADD_TEST(test_ln_strcat_delim_alloc);
    LN_TEST_ADD_TEST(test_ln_next_multiple_power2);
    LN_TEST_ADD_TEST(test_ln_autopadding_conv);
    LN_TEST_ADD_TEST(test_ln_autopadding_deconv);
    LN_TEST_ADD_TEST(test_ln_subfixed);
    LN_TEST_ADD_TEST(test_ln_is_prefix_plus_digit);
}
LN_TEST_TCASE_END

LN_TEST_ADD_TCASE(util);

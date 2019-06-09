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
     /* end of adding tests */

     suite_add_tcase(s, tc_util);

     return s;
}

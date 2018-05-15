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

#include "test_lightnet.h"
#include "../src/ln_param.h"

static void setup(void)
{
}

static void teardown(void)
{
}

START_TEST(test_ln_param_table_append_string)
{
     ln_param_table *params = NULL;
     ln_param_entry *entry;

     params = ln_param_table_append_string(params, "test_arg_name_1", "test_string_1");
     params = ln_param_table_append_string(params, "test_arg_name_2", "test_string_2");
     entry = ln_param_table_find_by_arg_name(params, "test_arg_name_1");
     ck_assert_str_eq(entry->value_string, "test_string_1");
     entry = ln_param_table_find_by_arg_name(params, "test_arg_name_2");
     ck_assert_str_eq(entry->value_string, "test_string_2");
     ln_param_table_free(params);
}
END_TEST

START_TEST(test_ln_param_table_append_number)
{
     ln_param_table *params = NULL;
     ln_param_entry *entry;

     params = ln_param_table_append_number(params, "test_arg_name_1", 1);
     params = ln_param_table_append_number(params, "test_arg_name_2", 2);
     entry = ln_param_table_find_by_arg_name(params, "test_arg_name_1");
     ck_assert_int_eq(entry->value_int, 1);
     entry = ln_param_table_find_by_arg_name(params, "test_arg_name_2");
     ck_assert_int_eq(entry->value_int, 2);
     ln_param_table_free(params);

     params = ln_param_table_append_number(params, "test_arg_name_1", 1.1);
     params = ln_param_table_append_number(params, "test_arg_name_2", 2.2);
     entry = ln_param_table_find_by_arg_name(params, "test_arg_name_1");
     ck_assert(entry->value_double == 1.1);
     entry = ln_param_table_find_by_arg_name(params, "test_arg_name_2");
     ck_assert(entry->value_double == 2.2);
     ln_param_table_free(params);
}
END_TEST

START_TEST(test_ln_param_table_append_bool)
{
}
END_TEST

START_TEST(test_ln_param_table_append_null)
{
}
END_TEST

START_TEST(test_ln_param_table_append_array_string)
{
}
END_TEST

START_TEST(test_ln_param_table_append_array_number)
{
}
END_TEST

START_TEST(test_ln_param_table_append_array_bool)
{
}
END_TEST

START_TEST(test_ln_param_table_find_by_arg_name)
{
}
END_TEST

START_TEST(test_ln_param_table_length)
{
}
END_TEST

START_TEST(test_ln_param_type_name)
{
}
END_TEST
/* end of tests */

Suite *make_param_suite(void)
{
     Suite *s;
     TCase *tc_param;

     s = suite_create("param");
     tc_param = tcase_create("param");
     tcase_add_checked_fixture(tc_param, setup, teardown);

     tcase_add_test(tc_param, test_ln_param_table_append_string);
     tcase_add_test(tc_param, test_ln_param_table_append_number);
     tcase_add_test(tc_param, test_ln_param_table_append_bool);
     tcase_add_test(tc_param, test_ln_param_table_append_null);
     tcase_add_test(tc_param, test_ln_param_table_append_array_string);
     tcase_add_test(tc_param, test_ln_param_table_append_array_number);
     tcase_add_test(tc_param, test_ln_param_table_append_array_bool);
     tcase_add_test(tc_param, test_ln_param_table_find_by_arg_name);
     tcase_add_test(tc_param, test_ln_param_table_length);
     tcase_add_test(tc_param, test_ln_param_type_name);
     /* end of adding tests */

     suite_add_tcase(s, tc_param);

     return s;
}

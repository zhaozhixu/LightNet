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

START_TEST(test_ln_param_list_append_string)
{
     ln_list *params;
     ln_param_entry *entry;

     params = ln_param_list_append_string(NULL, "test_arg_name_1", "test_string_1");
     params = ln_param_list_append_string(params, "test_arg_name_2", "test_string_2");
     ck_assert_int_eq(ln_param_list_length(params), 2);

     entry = ln_param_list_find_by_arg_name(params, "test_arg_name_1");
     ck_assert_int_eq(entry->type, LN_PARAM_STRING);
     ck_assert_str_eq(entry->value_string, "test_string_1");
     entry = ln_param_list_find_by_arg_name(params, "test_arg_name_2");
     ck_assert_int_eq(entry->type, LN_PARAM_STRING);
     ck_assert_str_eq(entry->value_string, "test_string_2");
     ln_param_list_free(params);
}
END_TEST

START_TEST(test_ln_param_list_append_number)
{
     ln_list *params;
     ln_param_entry *entry;

     params = ln_param_list_append_number(NULL, "test_arg_name_1", 1);
     params = ln_param_list_append_number(params, "test_arg_name_2", 2);
     ck_assert_int_eq(ln_param_list_length(params), 2);

     entry = ln_param_list_find_by_arg_name(params, "test_arg_name_1");
     ck_assert_int_eq(entry->type, LN_PARAM_NUMBER);
     ck_assert_int_eq(entry->value_int, 1);
     entry = ln_param_list_find_by_arg_name(params, "test_arg_name_2");
     ck_assert_int_eq(entry->type, LN_PARAM_NUMBER);
     ck_assert_int_eq(entry->value_int, 2);
     ln_param_list_free(params);

     params = ln_param_list_append_number(NULL, "test_arg_name_1", 1.1);
     params = ln_param_list_append_number(params, "test_arg_name_2", 2.2);
     ck_assert_int_eq(ln_param_list_length(params), 2);

     entry = ln_param_list_find_by_arg_name(params, "test_arg_name_1");
     ck_assert_int_eq(entry->type, LN_PARAM_NUMBER);
     ck_assert(entry->value_double == 1.1);
     entry = ln_param_list_find_by_arg_name(params, "test_arg_name_2");
     ck_assert_int_eq(entry->type, LN_PARAM_NUMBER);
     ck_assert(entry->value_double == 2.2);
     ln_param_list_free(params);
}
END_TEST

START_TEST(test_ln_param_list_append_bool)
{
     ln_list *params;
     ln_param_entry *entry;

     params = ln_param_list_append_bool(NULL, "test_arg_name_1", LN_FALSE);
     params = ln_param_list_append_bool(params, "test_arg_name_2", LN_TRUE);
     ck_assert_int_eq(ln_param_list_length(params), 2);

     entry = ln_param_list_find_by_arg_name(params, "test_arg_name_1");
     ck_assert_int_eq(entry->type, LN_PARAM_BOOL);
     ck_assert_int_eq(entry->value_bool, LN_FALSE);
     entry = ln_param_list_find_by_arg_name(params, "test_arg_name_2");
     ck_assert_int_eq(entry->type, LN_PARAM_BOOL);
     ck_assert_int_eq(entry->value_bool, LN_TRUE);
     ln_param_list_free(params);

}
END_TEST

START_TEST(test_ln_param_list_append_null)
{
     ln_list *params;
     ln_param_entry *entry;

     params = ln_param_list_append_null(NULL, "test_arg_name_1");
     params = ln_param_list_append_null(params, "test_arg_name_2");
     ck_assert_int_eq(ln_param_list_length(params), 2);

     entry = ln_param_list_find_by_arg_name(params, "test_arg_name_1");
     ck_assert_int_eq(entry->type, LN_PARAM_NULL);
     entry = ln_param_list_find_by_arg_name(params, "test_arg_name_2");
     ck_assert_int_eq(entry->type, LN_PARAM_NULL);
     ln_param_list_free(params);
}
END_TEST

START_TEST(test_ln_param_list_append_array_string)
{
     ln_list *params;
     ln_param_entry *entry;
     int array_len1 = 2, array_len2 = 2;
     char *array_string1[] = {"test_array1_str1", "test_array1_str2"};
     char *array_string2[] = {"test_array2_str1", "test_array2_str2"};

     params = ln_param_list_append_array_string(NULL, "test_arg_name_1",
						 array_len1, array_string1);
     params = ln_param_list_append_array_string(params, "test_arg_name_2",
						 array_len2, array_string2);
     ck_assert_int_eq(ln_param_list_length(params), 2);

     entry = ln_param_list_find_by_arg_name(params, "test_arg_name_1");
     ck_assert_int_eq(entry->type, LN_PARAM_ARRAY_STRING);
     ck_assert_str_eq(entry->value_array_string[0], array_string1[0]);
     ck_assert_str_eq(entry->value_array_string[1], array_string1[1]);
     entry = ln_param_list_find_by_arg_name(params, "test_arg_name_2");
     ck_assert_int_eq(entry->type, LN_PARAM_ARRAY_STRING);
     ck_assert_str_eq(entry->value_array_string[0], array_string2[0]);
     ck_assert_str_eq(entry->value_array_string[1], array_string2[1]);
     ln_param_list_free(params);
}
END_TEST

START_TEST(test_ln_param_list_append_array_number)
{
     ln_list *params;
     ln_param_entry *entry;
     int array_len1 = 2, array_len2 = 2;
     double array_int1[] = {0, 1};
     double array_int2[] = {2, 3};
     double array_double1[] = {0.0, 1.1};
     double array_double2[] = {2.2, 3.3};

     params = ln_param_list_append_array_number(NULL, "test_arg_name_1",
						 array_len1, array_int1);
     params = ln_param_list_append_array_number(params, "test_arg_name_2",
						 array_len2, array_int2);
     ck_assert_int_eq(ln_param_list_length(params), 2);

     entry = ln_param_list_find_by_arg_name(params, "test_arg_name_1");
     ck_assert_int_eq(entry->type, LN_PARAM_ARRAY_NUMBER);
     ck_assert_int_eq(entry->value_array_int[0], (int)array_int1[0]);
     ck_assert_int_eq(entry->value_array_int[1], (int)array_int1[1]);
     entry = ln_param_list_find_by_arg_name(params, "test_arg_name_2");
     ck_assert_int_eq(entry->type, LN_PARAM_ARRAY_NUMBER);
     ck_assert_int_eq(entry->value_array_int[0], (int)array_int2[0]);
     ck_assert_int_eq(entry->value_array_int[1], (int)array_int2[1]);
     ln_param_list_free(params);

     params = ln_param_list_append_array_number(NULL, "test_arg_name_1",
						 array_len1, array_double1);
     params = ln_param_list_append_array_number(params, "test_arg_name_2",
						 array_len2, array_double2);
     ck_assert_int_eq(ln_param_list_length(params), 2);

     entry = ln_param_list_find_by_arg_name(params, "test_arg_name_1");
     ck_assert_int_eq(entry->type, LN_PARAM_ARRAY_NUMBER);
     ck_assert(entry->value_array_double[0] == array_double1[0]);
     ck_assert(entry->value_array_double[1] == array_double1[1]);
     entry = ln_param_list_find_by_arg_name(params, "test_arg_name_2");
     ck_assert_int_eq(entry->type, LN_PARAM_ARRAY_NUMBER);
     ck_assert(entry->value_array_double[0] == array_double2[0]);
     ck_assert(entry->value_array_double[1] == array_double2[1]);
     ln_param_list_free(params);
}
END_TEST

START_TEST(test_ln_param_list_append_array_bool)
{
     ln_list *params;
     ln_param_entry *entry;
     int array_len1 = 2, array_len2 = 2;
     ln_bool array_bool1[] = {LN_FALSE, LN_TRUE};
     ln_bool array_bool2[] = {LN_TRUE, LN_FALSE};

     params = ln_param_list_append_array_bool(NULL, "test_arg_name_1",
						 array_len1, array_bool1);
     params = ln_param_list_append_array_bool(params, "test_arg_name_2",
						 array_len2, array_bool2);
     ck_assert_int_eq(ln_param_list_length(params), 2);

     entry = ln_param_list_find_by_arg_name(params, "test_arg_name_1");
     ck_assert_int_eq(entry->type, LN_PARAM_ARRAY_BOOL);
     ck_assert_int_eq(entry->value_array_bool[0], array_bool1[0]);
     ck_assert_int_eq(entry->value_array_bool[1], array_bool1[1]);
     entry = ln_param_list_find_by_arg_name(params, "test_arg_name_2");
     ck_assert_int_eq(entry->type, LN_PARAM_ARRAY_BOOL);
     ck_assert_int_eq(entry->value_array_bool[0], array_bool2[0]);
     ck_assert_int_eq(entry->value_array_bool[1], array_bool2[1]);
     ln_param_list_free(params);
}
END_TEST

START_TEST(test_ln_param_list_find_by_arg_name)
{
}
END_TEST

START_TEST(test_ln_param_list_length)
{
}
END_TEST

START_TEST(test_ln_param_type_name)
{
     ck_assert_str_eq(ln_param_type_name(LN_PARAM_NULL), "null");
     ck_assert_str_eq(ln_param_type_name(LN_PARAM_STRING), "String");
     ck_assert_str_eq(ln_param_type_name(LN_PARAM_NUMBER), "Number");
     ck_assert_str_eq(ln_param_type_name(LN_PARAM_BOOL), "Boolean");
     ck_assert_str_eq(ln_param_type_name(LN_PARAM_ARRAY_STRING),
		      "String Array");
     ck_assert_str_eq(ln_param_type_name(LN_PARAM_ARRAY_NUMBER),
		      "Number Array");
     ck_assert_str_eq(ln_param_type_name(LN_PARAM_ARRAY_BOOL),
		      "Boolean Array");
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

     tcase_add_test(tc_param, test_ln_param_list_append_string);
     tcase_add_test(tc_param, test_ln_param_list_append_number);
     tcase_add_test(tc_param, test_ln_param_list_append_bool);
     tcase_add_test(tc_param, test_ln_param_list_append_null);
     tcase_add_test(tc_param, test_ln_param_list_append_array_string);
     tcase_add_test(tc_param, test_ln_param_list_append_array_number);
     tcase_add_test(tc_param, test_ln_param_list_append_array_bool);
     tcase_add_test(tc_param, test_ln_param_list_find_by_arg_name);
     tcase_add_test(tc_param, test_ln_param_list_length);
     tcase_add_test(tc_param, test_ln_param_type_name);
     /* end of adding tests */

     suite_add_tcase(s, tc_param);

     return s;
}

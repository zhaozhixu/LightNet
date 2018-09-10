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
#include "../src/ln_tensor.h"

static void setup(void)
{
}

static void teardown(void)
{
}

START_TEST(test_ln_tensor_list_append)
{
     ln_list *tensors;
     ln_tensor_entry *entry;

     tensors = ln_tensor_list_append(NULL, "test_arg_name1", "test_name1");
     tensors = ln_tensor_list_append(tensors, "test_arg_name2", "test_name2");
     ck_assert_int_eq(ln_tensor_list_length(tensors), 2);

     entry = ln_tensor_list_find_by_arg_name(tensors, "test_arg_name1");
     ck_assert_str_eq(entry->arg_name, "test_arg_name1");
     ck_assert_str_eq(entry->name, "test_name1");
     ck_assert_int_eq(entry->mtype, LN_MEM_UNDEFINED);
     ck_assert_ptr_eq(entry->tensor, NULL);
     entry = ln_tensor_list_find_by_arg_name(tensors, "test_arg_name2");
     ck_assert_str_eq(entry->arg_name, "test_arg_name2");
     ck_assert_str_eq(entry->name, "test_name2");
     ck_assert_int_eq(entry->mtype, LN_MEM_UNDEFINED);
     ck_assert_ptr_eq(entry->tensor, NULL);

     entry = ln_tensor_list_find_by_name(tensors, "test_name1");
     ck_assert_str_eq(entry->arg_name, "test_arg_name1");
     ck_assert_str_eq(entry->name, "test_name1");
     ck_assert_int_eq(entry->mtype, LN_MEM_UNDEFINED);
     ck_assert_ptr_eq(entry->tensor, NULL);
     entry = ln_tensor_list_find_by_name(tensors, "test_name2");
     ck_assert_str_eq(entry->arg_name, "test_arg_name2");
     ck_assert_str_eq(entry->name, "test_name2");
     ck_assert_int_eq(entry->mtype, LN_MEM_UNDEFINED);
     ck_assert_ptr_eq(entry->tensor, NULL);

     ln_tensor_list_free(tensors);
}
END_TEST

START_TEST(test_ln_tensor_list_free)
{
}
END_TEST

START_TEST(test_ln_tensor_list_find_by_arg_name)
{
}
END_TEST

START_TEST(test_ln_tensor_list_find_by_name)
{
}
END_TEST

START_TEST(test_ln_tensor_list_length)
{
}
END_TEST
/* end of tests */

Suite *make_tensor_suite(void)
{
     Suite *s;
     TCase *tc_tensor;

     s = suite_create("tensor");
     tc_tensor = tcase_create("tensor");
     tcase_add_checked_fixture(tc_tensor, setup, teardown);

     tcase_add_test(tc_tensor, test_ln_tensor_list_append);
     tcase_add_test(tc_tensor, test_ln_tensor_list_free);
     tcase_add_test(tc_tensor, test_ln_tensor_list_find_by_arg_name);
     tcase_add_test(tc_tensor, test_ln_tensor_list_find_by_name);
     tcase_add_test(tc_tensor, test_ln_tensor_list_length);
     /* end of adding tests */

     suite_add_tcase(s, tc_tensor);

     return s;
}

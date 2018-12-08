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

#define ARR(type, varg...) (type[]){varg}

static void checked_setup(void)
{
}

static void checked_teardown(void)
{
}

START_TEST(test_ln_tensor_list)
{
     ln_list *tensors;
     char *name;

     tensors = ln_tensor_list_append(NULL, "arg_name1", "name1");
     tensors = ln_tensor_list_append(tensors, "arg_name2", "name2");
     ck_assert_int_eq(ln_tensor_list_length(tensors), 2);

     name = ln_tensor_list_find_name(tensors, "arg_name1");
     ck_assert_str_eq(name, "name1");
     name = ln_tensor_list_find_name(tensors, "arg_name2");
     ck_assert_str_eq(name, "name2");
     name = ln_tensor_list_find_name(tensors, "not_exist");
     ck_assert_ptr_eq(name, NULL);

     ln_tensor_list_free(tensors);
}
END_TEST

START_TEST(test_ln_tensor_table)
{
     ln_hash *table;
     ln_tensor_entry *e, *e1, *e2;
     tl_tensor *t1, *t2;
     int ret;

     t1 = tl_tensor_create(NULL, 2, ARR(int,2,3), TL_INT32);
     t2 = tl_tensor_create(NULL, 2, ARR(int,4,5), TL_INT32);
     e1 = ln_tensor_entry_create("t1", t1);
     e2 = ln_tensor_entry_create("t2", t2);
     table = ln_tensor_table_create();

     ret = ln_tensor_table_insert(table, e1);
     ck_assert_int_eq(ret, 1);
     ret = ln_tensor_table_insert(table, e2);
     ck_assert_int_eq(ret, 1);
     ret = ln_tensor_table_insert(table, e2);
     ck_assert_int_eq(ret, 1);

     e = ln_tensor_table_find(table, "t1");
     ck_assert_str_eq(e->name, "t1");
     ck_assert_ptr_eq(e->tensor, t1);
     e = ln_tensor_table_find(table, "t2");
     ck_assert_str_eq(e->name, "t2");
     ck_assert_ptr_eq(e->tensor, t2);
     e = ln_tensor_table_find(table, "t3");
     ck_assert_ptr_eq(e, NULL);

     ret = ln_tensor_table_remove(table, "t1");
     ck_assert_int_eq(ret, 1);
     e = ln_tensor_table_find(table, "t1");
     ck_assert_ptr_eq(e, NULL);
     ret = ln_tensor_table_remove(table, "t2");
     ck_assert_int_eq(ret, 1);
     e = ln_tensor_table_find(table, "t2");
     ck_assert_ptr_eq(e, NULL);
     ret = ln_tensor_table_remove(table, "t3");
     ck_assert_int_eq(ret, 0);

     ln_tensor_table_free(table);
}
END_TEST
/* end of tests */

Suite *make_tensor_suite(void)
{
     Suite *s;
     TCase *tc_tensor;

     s = suite_create("tensor");
     tc_tensor = tcase_create("tensor");
     tcase_add_checked_fixture(tc_tensor, checked_setup, checked_teardown);

     tcase_add_test(tc_tensor, test_ln_tensor_list);
     tcase_add_test(tc_tensor, test_ln_tensor_table);
     /* end of adding tests */

     suite_add_tcase(s, tc_tensor);

     return s;
}

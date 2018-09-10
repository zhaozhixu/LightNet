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

#include <assert.h>
#include "test_lightnet.h"
#include "../src/ln_op.h"

static void setup(void)
{
}

static void teardown(void)
{
}

static void pre_run (ln_op_arg *op_arg, ln_error **error)
{
}

static void run (ln_op_arg *op_arg, ln_error **error)
{
}

static void post_run (ln_op_arg *op_arg, ln_error **error)
{
}

static void pre_run1 (ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *tensor_entry;
     int tensors_n, params_n;

     tensors_n = ln_tensor_list_length(op_arg->tensors_in);
     ln_op_check_tensor_in_len_eq(LN_ERROR, tensors_n, 3);

     tensor_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "test_tensor_arg_name1");
     ln_op_check_tensor_in_exist(LN_ERROR, tensor_entry, "test_tensor_arg_name1");
     ln_op_check_tensor_defined(LN_ERROR, tensor_entry);

     tensor_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "test_tensor_arg_name2");
     ln_op_check_tensor_in_exist(LN_ERROR, tensor_entry, "test_tensor_arg_name2");
     ln_op_check_tensor_defined(LN_ERROR, tensor_entry);

     tensor_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "test_tensor_arg_name5");
     ln_op_check_tensor_in_exist(LN_ERROR, tensor_entry, "test_tensor_arg_name5");
     ln_op_check_tensor_not_defined(LN_WARNING, tensor_entry);

     params_n = ln_param_list_length(op_arg->params);
     ln_op_check_param_len_eq(LN_ERROR, params_n, 0);

     tensor_entry->tensor = tl_tensor_zeros(2, (int[]){1, 2}, TL_INT32);
}

static void run1 (ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *tensor_entry;

     tensor_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "test_tensor_arg_name5");
     assert(tensor_entry);

     ((int32_t *)tensor_entry->tensor->data)[0] = 1;
     ((int32_t *)tensor_entry->tensor->data)[1] = 1;
}

static void post_run1 (ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *tensor_entry;

     tensor_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "test_tensor_arg_name5");
     assert(tensor_entry);

     tl_tensor_free_data_too(tensor_entry->tensor);
     tensor_entry->tensor = NULL;
}

static void pre_run2 (ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *tensor_entry;
     int tensors_n, params_n;

     tensors_n = ln_tensor_list_length(op_arg->tensors_out);
     ln_op_check_tensor_out_len_eq(LN_ERROR, tensors_n, 3);

     tensor_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "test_tensor_arg_name3");
     ln_op_check_tensor_out_exist(LN_ERROR, tensor_entry, "test_tensor_arg_name3");
     ln_op_check_tensor_defined(LN_ERROR, tensor_entry);

     tensor_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "test_tensor_arg_name4");
     ln_op_check_tensor_out_exist(LN_ERROR, tensor_entry, "test_tensor_arg_name4");
     ln_op_check_tensor_defined(LN_ERROR, tensor_entry);

     tensor_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "test_tensor_arg_name6");
     ln_op_check_tensor_out_exist(LN_ERROR, tensor_entry, "test_tensor_arg_name6");
     ln_op_check_tensor_not_defined(LN_WARNING, tensor_entry);

     params_n = ln_param_list_length(op_arg->params);
     ln_op_check_param_len_eq(LN_ERROR, params_n, 0);

     tensor_entry->tensor = tl_tensor_zeros(2, (int[]){1, 2}, TL_INT32);
}

static void run2 (ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *tensor_entry;

     tensor_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "test_tensor_arg_name6");
     assert(tensor_entry);

     ((int32_t *)tensor_entry->tensor->data)[0] = 1;
     ((int32_t *)tensor_entry->tensor->data)[1] = 1;
}

static void post_run2 (ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *tensor_entry;

     tensor_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "test_tensor_arg_name6");
     assert(tensor_entry);

     tl_tensor_free_data_too(tensor_entry->tensor);
     tensor_entry->tensor = NULL;
}

START_TEST(test_ln_op_create)
{
     ln_op *op;
     ln_list *tensors;
     ln_list *params;

     params = ln_param_list_append_string(NULL, "test_params_arg_name1",
                                          "test_params_string1");
     params = ln_param_list_append_string(params, "test_params_arg_name2",
                                          "test_params_string2");
     tensors = ln_tensor_list_append(NULL, "test_tensor_arg_name1",
                                     "test_tensor_name1");
     tensors = ln_tensor_list_append(tensors, "test_tensor_arg_name2",
                                     "test_tensor_name2");

     op = ln_op_create("test_name", "test_optype", tensors, NULL,
                       params, pre_run, run, post_run);
     ck_assert_ptr_eq(op->pre_run, pre_run);
     ck_assert_ptr_eq(op->run, run);
     ck_assert_ptr_eq(op->post_run, post_run);
     ck_assert_str_eq(op->op_arg->name, "test_name");
     ck_assert_str_eq(op->op_arg->optype, "test_optype");
     ck_assert_ptr_eq(op->op_arg->params, params);
     ck_assert_ptr_eq(op->op_arg->tensors_in, tensors);
     ck_assert_ptr_eq(op->op_arg->tensors_out, NULL);

     ln_tensor_list_free(tensors);
     ln_param_list_free(params);
     ln_op_free(op);
}
END_TEST

START_TEST(test_ln_op_free)
{
}
END_TEST

START_TEST(test_ln_op_list_free_lists_too)
{
}
END_TEST

START_TEST(test_ln_op_list_find_by_optype)
{
}
END_TEST

START_TEST(test_ln_op_list_do_pre_run)
{
}
END_TEST

START_TEST(test_ln_op_list_do_run)
{
}
END_TEST

START_TEST(test_ln_op_list_do_post_run)
{
}
END_TEST
/* end of tests */

Suite *make_op_suite(void)
{
     Suite *s;
     TCase *tc_op;

     s = suite_create("op");
     tc_op = tcase_create("op");
     tcase_add_checked_fixture(tc_op, setup, teardown);

     tcase_add_test(tc_op, test_ln_op_create);
     tcase_add_test(tc_op, test_ln_op_free);
     tcase_add_test(tc_op, test_ln_op_list_free_lists_too);
     tcase_add_test(tc_op, test_ln_op_list_find_by_optype);
     tcase_add_test(tc_op, test_ln_op_list_do_pre_run);
     tcase_add_test(tc_op, test_ln_op_list_do_run);
     tcase_add_test(tc_op, test_ln_op_list_do_post_run);
     /* end of adding tests */

     suite_add_tcase(s, tc_op);

     return s;
}

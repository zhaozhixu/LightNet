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

     tensors_n = ln_tensor_table_length(op_arg->tensors_in);
     ln_op_check_tensor_in_len_eq(LN_ERROR, tensors_n, 3);

     tensor_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors_in, "test_tensor_arg_name1");
     ln_op_check_tensor_in_exist(LN_ERROR, tensor_entry, "test_tensor_arg_name1");
     ln_op_check_tensor_defined(LN_ERROR, tensor_entry);

     tensor_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors_in, "test_tensor_arg_name2");
     ln_op_check_tensor_in_exist(LN_ERROR, tensor_entry, "test_tensor_arg_name2");
     ln_op_check_tensor_defined(LN_ERROR, tensor_entry);

     tensor_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors_in, "test_tensor_arg_name5");
     ln_op_check_tensor_in_exist(LN_ERROR, tensor_entry, "test_tensor_arg_name5");
     ln_op_check_tensor_not_defined(LN_WARNING, tensor_entry);

     params_n = ln_param_table_length(op_arg->params);
     ln_op_check_param_len_eq(LN_ERROR, params_n, 0);

     tensor_entry->tensor = tl_tensor_zeros(2, (int[]){1, 2}, TL_INT32);
}

static void run1 (ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *tensor_entry;

     tensor_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors_in, "test_tensor_arg_name5");
     assert(tensor_entry);

     ((int32_t *)tensor_entry->tensor->data)[0] = 1;
     ((int32_t *)tensor_entry->tensor->data)[1] = 1;
}

static void post_run1 (ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *tensor_entry;

     tensor_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors_in, "test_tensor_arg_name5");
     assert(tensor_entry);

     tl_tensor_free_data_too(tensor_entry->tensor);
     tensor_entry->tensor = NULL;
}

static void pre_run2 (ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *tensor_entry;
     int tensors_n, params_n;

     tensors_n = ln_tensor_table_length(op_arg->tensors_out);
     ln_op_check_tensor_out_len_eq(LN_ERROR, tensors_n, 3);

     tensor_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors_out, "test_tensor_arg_name3");
     ln_op_check_tensor_out_exist(LN_ERROR, tensor_entry, "test_tensor_arg_name3");
     ln_op_check_tensor_defined(LN_ERROR, tensor_entry);

     tensor_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors_out, "test_tensor_arg_name4");
     ln_op_check_tensor_out_exist(LN_ERROR, tensor_entry, "test_tensor_arg_name4");
     ln_op_check_tensor_defined(LN_ERROR, tensor_entry);

     tensor_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors_out, "test_tensor_arg_name6");
     ln_op_check_tensor_out_exist(LN_ERROR, tensor_entry, "test_tensor_arg_name6");
     ln_op_check_tensor_not_defined(LN_WARNING, tensor_entry);

     params_n = ln_param_table_length(op_arg->params);
     ln_op_check_param_len_eq(LN_ERROR, params_n, 0);

     tensor_entry->tensor = tl_tensor_zeros(2, (int[]){1, 2}, TL_INT32);
}

static void run2 (ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *tensor_entry;

     tensor_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors_out, "test_tensor_arg_name6");
     assert(tensor_entry);

     ((int32_t *)tensor_entry->tensor->data)[0] = 1;
     ((int32_t *)tensor_entry->tensor->data)[1] = 1;
}

static void post_run2 (ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *tensor_entry;

     tensor_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors_out, "test_tensor_arg_name6");
     assert(tensor_entry);

     tl_tensor_free_data_too(tensor_entry->tensor);
     tensor_entry->tensor = NULL;
}

START_TEST(test_ln_op_create)
{
     ln_op *op;
     ln_tensor_table *tensors;
     tl_tensor *tensor1, *tensor2;
     ln_param_table *params;

     params = ln_param_table_append_string(NULL, "test_params_arg_name1",
                                           "test_params_string1");
     params = ln_param_table_append_string(params, "test_params_arg_name2",
                                           "test_params_string2");
     tensor1 = tl_tensor_zeros(2, (int[]){1, 2}, TL_INT32);
     tensor2 = tl_tensor_zeros(2, (int[]){3, 4}, TL_INT32);
     tensors = ln_tensor_table_append(NULL, "test_tensor_arg_name1",
                                      "test_tensor_name1", LN_MEM_CPU, tensor1);
     tensors = ln_tensor_table_append(tensors, "test_tensor_arg_name2",
                                      "test_tensor_name2", LN_MEM_CPU, tensor2);

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

     tl_tensor_free_data_too(tensor1);
     tl_tensor_free_data_too(tensor2);
     ln_tensor_table_free(tensors);
     ln_param_table_free(params);
     ln_op_free(op);
}
END_TEST

START_TEST(test_ln_op_free)
{
}
END_TEST

START_TEST(test_ln_op_list_free_tables_too)
{
}
END_TEST

START_TEST(test_ln_op_list_find_tensor_by_name)
{
     ln_list *ops;
     ln_op *op1, *op2, *op;
     ln_tensor_table *tensors;
     tl_tensor *tensor1, *tensor2, *tensor3, *tensor4, *tensor;

     tensor1 = tl_tensor_zeros(2, (int[]){1, 2}, TL_INT32);
     tensor2 = tl_tensor_zeros(2, (int[]){1, 2}, TL_INT32);
     tensors = ln_tensor_table_append(NULL, "test_tensor_arg_name1",
                                      "test_tensor_name1", LN_MEM_CPU, tensor1);
     tensors = ln_tensor_table_append(tensors, "test_tensor_arg_name2",
                                      "test_tensor_name2", LN_MEM_CPU, tensor2);
     tensors = ln_tensor_table_append(tensors, "test_tensor_arg_name5",
                                      "test_tensor_name5", LN_MEM_CPU, NULL);
     op1 = ln_op_create("test_name1", "test_optype1", tensors, NULL,
                        NULL, pre_run1, run1, post_run1);

     tensor3 = tl_tensor_zeros(2, (int[]){1, 2}, TL_INT32);
     tensor4 = tl_tensor_zeros(2, (int[]){1, 2}, TL_INT32);
     tensors = ln_tensor_table_append(NULL, "test_tensor_arg_name3",
                                      "test_tensor_name3", LN_MEM_CPU, tensor3);
     tensors = ln_tensor_table_append(tensors, "test_tensor_arg_name4",
                                      "test_tensor_name4", LN_MEM_CPU, tensor4);
     tensors = ln_tensor_table_append(tensors, "test_tensor_arg_name6",
                                      "test_tensor_name6", LN_MEM_CPU, NULL);
     op2 = ln_op_create("test_name2", "test_optype2", NULL,
                        tensors, NULL, pre_run2, run2, post_run2);

     ops = ln_list_append(NULL, op1);
     ops = ln_list_append(ops, op2);

     tensor = ln_op_list_find_tensor_by_name(ops, "test_tensor_name1");
     ck_assert_ptr_eq(tensor, tensor1);
     tensor = ln_op_list_find_tensor_by_name(ops, "test_tensor_name2");
     ck_assert_ptr_eq(tensor, tensor2);
     tensor = ln_op_list_find_tensor_by_name(ops, "test_tensor_name3");
     ck_assert_ptr_eq(tensor, tensor3);
     tensor = ln_op_list_find_tensor_by_name(ops, "test_tensor_name4");
     ck_assert_ptr_eq(tensor, tensor4);
     ck_assert_ptr_eq(ln_op_list_find_tensor_by_name(ops, "test_tensor_name5"), NULL);
     ck_assert_ptr_eq(ln_op_list_find_tensor_by_name(ops, "test_tensor_name6"), NULL);

     op = ln_op_list_find_by_optype(ops, "test_optype1");
     ck_assert_ptr_eq(op, op1);
     op = ln_op_list_find_by_optype(ops, "test_optype2");
     ck_assert_ptr_eq(op, op2);

     ln_error *error = NULL;
     ln_op_list_do_pre_run(ops, &error);
     ln_error_handle(&error);
     ck_assert_ptr_ne(ln_op_list_find_tensor_by_name(ops, "test_tensor_name5"), NULL);
     ck_assert_ptr_ne(ln_op_list_find_tensor_by_name(ops, "test_tensor_name6"), NULL);
     ck_assert_int_eq(((int32_t *)ln_op_list_find_tensor_by_name(ops, "test_tensor_name5")->data)[0], 0);
     ck_assert_int_eq(((int32_t *)ln_op_list_find_tensor_by_name(ops, "test_tensor_name5")->data)[0], 0);
     ck_assert_int_eq(((int32_t *)ln_op_list_find_tensor_by_name(ops, "test_tensor_name6")->data)[0], 0);
     ck_assert_int_eq(((int32_t *)ln_op_list_find_tensor_by_name(ops, "test_tensor_name6")->data)[0], 0);

     ln_op_list_do_run(ops, &error);
     ln_error_handle(&error);
     ck_assert_int_eq(((int32_t *)ln_op_list_find_tensor_by_name(ops, "test_tensor_name5")->data)[0], 1);
     ck_assert_int_eq(((int32_t *)ln_op_list_find_tensor_by_name(ops, "test_tensor_name5")->data)[0], 1);
     ck_assert_int_eq(((int32_t *)ln_op_list_find_tensor_by_name(ops, "test_tensor_name6")->data)[0], 1);
     ck_assert_int_eq(((int32_t *)ln_op_list_find_tensor_by_name(ops, "test_tensor_name6")->data)[0], 1);

     ln_op_list_do_post_run(ops, &error);
     ln_error_handle((&error));
     ck_assert_ptr_eq(ln_op_list_find_tensor_by_name(ops, "test_tensor_name5"), NULL);
     ck_assert_ptr_eq(ln_op_list_find_tensor_by_name(ops, "test_tensor_name6"), NULL);

     tl_tensor_free_data_too(tensor1);
     tl_tensor_free_data_too(tensor2);
     tl_tensor_free_data_too(tensor3);
     tl_tensor_free_data_too(tensor4);
     ln_op_list_free_tables_too(ops);
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
     tcase_add_test(tc_op, test_ln_op_list_free_tables_too);
     tcase_add_test(tc_op, test_ln_op_list_find_tensor_by_name);
     tcase_add_test(tc_op, test_ln_op_list_find_by_optype);
     tcase_add_test(tc_op, test_ln_op_list_do_pre_run);
     tcase_add_test(tc_op, test_ln_op_list_do_run);
     tcase_add_test(tc_op, test_ln_op_list_do_post_run);
     /* end of adding tests */

     suite_add_tcase(s, tc_op);

     return s;
}

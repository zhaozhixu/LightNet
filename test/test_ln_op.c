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

static int pre_run_count = 0;
static int static_run_count = 0;
static int run_count = 0;
static int post_run_count = 0;

static void pre_run0 (ln_op_arg *op_arg, ln_msg **error)
{
     pre_run_count++;
}

static void static_run0 (ln_op_arg *op_arg, ln_msg **error)
{
     static_run_count++;
     *error = ln_msg_create(LN_WARNING, "static_run0");
}

static void run0 (ln_op_arg *op_arg, ln_msg **error)
{
     run_count++;
}

static void post_run0 (ln_op_arg *op_arg, ln_msg **error)
{
     post_run_count++;
}

static void pre_run1 (ln_op_arg *op_arg, ln_msg **error)
{
     pre_run_count++;
     *error = ln_msg_create(LN_INFO, "pre_run1");
}

static void run1 (ln_op_arg *op_arg, ln_msg **error)
{
     run_count++;
}

static void post_run1 (ln_op_arg *op_arg, ln_msg **error)
{
     post_run_count++;
}

static void pre_run2 (ln_op_arg *op_arg, ln_msg **error)
{
     pre_run_count++;
}

static void run2 (ln_op_arg *op_arg, ln_msg **error)
{
     run_count++;
}

static void post_run2 (ln_op_arg *op_arg, ln_msg **error)
{
     post_run_count++;
}

static ln_op_arg op_arg0 = {
     .optype = "test_optype0",
     .arch = "none",
     .name = "test_opname0",
     .in_arg_names = NULL,
     .out_arg_names = NULL,
     .param_arg_names = NULL,
};

static ln_op opimpl0 = {
     .op_arg = &op_arg0,
     .pre_run = pre_run0,
     .static_run = static_run0,
     .run = run0,
     .post_run = post_run0,
};

static ln_op_arg op_arg1 = {
     .optype = "test_optype1",
     .name = "test_opname1",
     .arch = "none",
     .in_arg_names = NULL,
     .out_arg_names = NULL,
     .param_arg_names = NULL,
};

static ln_op opimpl1 = {
     .op_arg = &op_arg1,
     .pre_run = pre_run1,
     .static_run = NULL,
     .run = run1,
     .post_run = post_run1,
};

static ln_op_arg op_arg2 = {
     .optype = "test_optype2",
     .name = "test_opname2",
     .arch = "none",
     .in_arg_names = NULL,
     .out_arg_names = NULL,
     .param_arg_names = NULL,
};

static ln_op opimpl2 = {
     .op_arg = &op_arg2,
     .pre_run = pre_run2,
     .static_run = NULL,
     .run = run2,
     .post_run = post_run2,
};

static ln_op *op_array[] = {
     &opimpl0,
     &opimpl1,
     &opimpl2,
};

static ln_list *test_op_list;

static void checked_setup(void)
{
     test_op_list = ln_op_list_create_from_array(op_array);
}

static void checked_teardown(void)
{
     ln_op_list_free(test_op_list);
}

START_TEST(test_ln_op_list_create_from_array)
{
     ln_list *oplist;
     ln_op *op;

     oplist = ln_op_list_create_from_array(op_array);

     op = oplist->data;
     ck_assert_ptr_eq(op, &opimpl0);
     oplist = oplist->next;
     op = oplist->data;
     ck_assert_ptr_eq(op, &opimpl1);
     oplist = oplist->next;
     op = oplist->data;
     ck_assert_ptr_eq(op, &opimpl2);
     oplist = oplist->next;
     ck_assert_ptr_eq(oplist, NULL);

     ln_op_list_free(oplist);
}
END_TEST

START_TEST(test_ln_op_list_free)
{
}
END_TEST

START_TEST(test_ln_op_list_free_lists_too)
{
}
END_TEST

START_TEST(test_ln_op_list_find_by_optype)
{
     ln_op *op;

     op = ln_op_list_find_by_optype(test_op_list, op_arg0.optype);
     ck_assert_ptr_eq(op, &opimpl0);
     op = ln_op_list_find_by_optype(test_op_list, op_arg1.optype);
     ck_assert_ptr_eq(op, &opimpl1);
     op = ln_op_list_find_by_optype(test_op_list, op_arg2.optype);
     ck_assert_ptr_eq(op, &opimpl2);
     op = ln_op_list_find_by_optype(test_op_list, "not_exist");
     ck_assert_ptr_eq(op, NULL);
}
END_TEST

START_TEST(test_ln_op_list_find_by_name)
{
     ln_op *op;

     op = ln_op_list_find_by_name(test_op_list, op_arg0.name);
     ck_assert_ptr_eq(op, &opimpl0);
     op = ln_op_list_find_by_name(test_op_list, op_arg1.name);
     ck_assert_ptr_eq(op, &opimpl1);
     op = ln_op_list_find_by_name(test_op_list, op_arg2.name);
     ck_assert_ptr_eq(op, &opimpl2);
     op = ln_op_list_find_by_name(test_op_list, "not_exist");
     ck_assert_ptr_eq(op, NULL);
}
END_TEST

START_TEST(test_ln_op_create_from_proto)
{
     ln_op *op;
     ln_list *tensors;
     ln_list *params;
     ln_hash *tensor_table;

     tensors = ln_tensor_list_append(NULL, "tensor1", "name1");
     params = ln_param_list_append_string(NULL, "param1", "string1");
     tensor_table = ln_tensor_table_create();

     op = ln_op_create_from_proto(&opimpl0, "opname", NULL,
                                  tensors, params, tensor_table);

     ck_assert_ptr_eq(op->pre_run, pre_run0);
     ck_assert_ptr_eq(op->static_run, static_run0);
     ck_assert_ptr_eq(op->run, run0);
     ck_assert_ptr_eq(op->post_run, post_run0);
     ck_assert_str_eq(op->op_arg->name, "opname");
     ck_assert_str_eq(op->op_arg->optype, op_arg0.optype);
     ck_assert_ptr_eq(op->op_arg->params, params);
     ck_assert_ptr_eq(op->op_arg->tensors_in, NULL);
     ck_assert_ptr_eq(op->op_arg->tensors_out, tensors);
     ck_assert_ptr_eq(op->op_arg->priv, NULL);

     ln_tensor_list_free(tensors);
     ln_param_list_free(params);
     ln_tensor_table_free(tensor_table);
     ln_op_free(op);
}
END_TEST

START_TEST(test_ln_op_free)
{
}
END_TEST

START_TEST(test_ln_op_free_lists_too)
{
}
END_TEST

START_TEST(test_ln_op_list_do_pre_run)
{
     ln_msg *error = NULL;

     ln_op_list_do_pre_run(test_op_list, &error);
     ck_assert_int_eq(error->level, LN_INFO);
     ck_assert_str_eq(error->err_str, "pre_run1");
     ln_msg_free(error);
     ck_assert_int_eq(pre_run_count, 2);
}
END_TEST

START_TEST(test_ln_op_list_do_static_run)
{
     ln_msg *error = NULL;

     ln_op_list_do_static_run(test_op_list, &error);
     ck_assert_int_eq(error->level, LN_WARNING);
     ck_assert_str_eq(error->err_str, "static_run0");
     ln_msg_free(error);
     ck_assert_int_eq(static_run_count, 1);
}
END_TEST

START_TEST(test_ln_op_list_do_run)
{
     ln_msg *error = NULL;

     ln_op_list_do_run(test_op_list, &error);
     ck_assert_ptr_eq(error, NULL);
     ck_assert_int_eq(run_count, 3);
}
END_TEST

START_TEST(test_ln_op_list_do_post_run)
{
     ln_msg *error = NULL;

     ln_op_list_do_post_run(test_op_list, &error);
     ck_assert_ptr_eq(error, NULL);
     ck_assert_int_eq(post_run_count, 3);
}
END_TEST
/* end of tests */

Suite *make_op_suite(void)
{
     Suite *s;
     TCase *tc_op;

     s = suite_create("op");
     tc_op = tcase_create("op");
     tcase_add_checked_fixture(tc_op, checked_setup, checked_teardown);

     tcase_add_test(tc_op, test_ln_op_list_create_from_array);
     tcase_add_test(tc_op, test_ln_op_list_free);
     tcase_add_test(tc_op, test_ln_op_list_free_lists_too);
     tcase_add_test(tc_op, test_ln_op_list_find_by_optype);
     tcase_add_test(tc_op, test_ln_op_list_find_by_name);
     tcase_add_test(tc_op, test_ln_op_create_from_proto);
     tcase_add_test(tc_op, test_ln_op_free);
     tcase_add_test(tc_op, test_ln_op_free_lists_too);
     tcase_add_test(tc_op, test_ln_op_list_do_pre_run);
     tcase_add_test(tc_op, test_ln_op_list_do_static_run);
     tcase_add_test(tc_op, test_ln_op_list_do_run);
     tcase_add_test(tc_op, test_ln_op_list_do_post_run);
     /* end of adding tests */

     suite_add_tcase(s, tc_op);

     return s;
}

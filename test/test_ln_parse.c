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

#include <sys/stat.h>
#include "test_lightnet.h"
#include "../src/ln_op.h"
#include "../src/ln_parse.h"

static char *json_str;
static ln_list *registered_ops;
static ln_error *error = NULL;

extern ln_op *ln_init_ops[];

static void setup(void)
{
     struct stat buf;
     FILE *fp;
     size_t n;

     if (stat("test_ln_parse.json", &buf) < 0) {
          perror("Cannot stat test_ln_parse.json");
          exit(EXIT_FAILURE);
     }

     json_str = ln_alloc(buf.st_size);
     if (!(fp = fopen("test_ln_parse.json", "rb"))) {
          perror("Cannot open test_ln_parse.json");
          exit(EXIT_FAILURE);
     }
     n = fread(json_str, buf.st_size, 1, fp);
     if (n < 1 && ferror(fp)) {
          perror("Error reading test_ln_parse.json");
          exit(EXIT_FAILURE);
     }

     fclose(fp);

     registered_ops = ln_op_list_create_from_array(ln_init_ops);
}

static void teardown(void)
{
     ln_free(json_str);
}

START_TEST(test_ln_parse_ops)
{
     ln_list *ops;
     ln_op *op;
     tl_tensor *tensor1, *tensor2;

     ops = ln_parse_ops(json_str, registered_ops, &error);
     ln_error_handle(&error);

     /* zeros1 */
     op = ln_list_nth_data(ops, 0);
     ck_assert_ptr_eq(op->pre_run, ln_op_list_find_by_optype(registered_ops, "zeros")->pre_run);
     ck_assert_ptr_eq(op->run, ln_op_list_find_by_optype(registered_ops, "zeros")->run);
     ck_assert_ptr_eq(op->post_run, ln_op_list_find_by_optype(registered_ops, "zeros")->post_run);
     ck_assert_str_eq(op->op_arg->name, "zeros1");
     ck_assert_str_eq(op->op_arg->optype, "zeros");
     ck_assert_ptr_eq(op->op_arg->priv, NULL);
     ck_assert_str_eq(ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "dst")->name,
                      "zeros1");
     tensor1 = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "dst")->tensor;
     ck_assert_int_eq(ln_param_table_find_by_arg_name(op->op_arg->params, "dtype")->type,
                      LN_PARAM_STRING);
     ck_assert_str_eq(ln_param_table_find_by_arg_name(op->op_arg->params, "dtype")->value_string,
                      "TL_FLOAT");
     ck_assert_int_eq(ln_param_table_find_by_arg_name(op->op_arg->params, "dims")->type,
                      LN_PARAM_ARRAY_NUMBER);
     ck_assert_int_eq(ln_param_table_find_by_arg_name(op->op_arg->params,
                                                      "dims")->value_array_int[0], 2);
     ck_assert_int_eq(ln_param_table_find_by_arg_name(op->op_arg->params,
                                                      "dims")->value_array_int[1], 4);

     /* slice1 */
     op = ln_list_nth_data(ops, 1);
     ck_assert_ptr_eq(op->pre_run, ln_op_list_find_by_optype(registered_ops, "slice")->pre_run);
     ck_assert_ptr_eq(op->run, ln_op_list_find_by_optype(registered_ops, "slice")->run);
     ck_assert_ptr_eq(op->post_run, ln_op_list_find_by_optype(registered_ops, "slice")->post_run);
     ck_assert_str_eq(op->op_arg->name, "slice1");
     ck_assert_str_eq(op->op_arg->optype, "slice");
     ck_assert_ptr_eq(op->op_arg->priv, NULL);
     ck_assert_str_eq(ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "src")->name,
                      "zeros1");
     ck_assert_ptr_eq(ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "src")->tensor,
                      tensor1);
     ck_assert_str_eq(ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "dst")->name,
                      "slice1");
     tensor1 = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "dst")->tensor;
     ck_assert_int_eq(ln_param_table_find_by_arg_name(op->op_arg->params, "axis")->type,
                      LN_PARAM_NUMBER);
     ck_assert_int_eq(ln_param_table_find_by_arg_name(op->op_arg->params, "axis")->value_int, 1);
     ck_assert_int_eq(ln_param_table_find_by_arg_name(op->op_arg->params, "start")->type,
                      LN_PARAM_NUMBER);
     ck_assert_int_eq(ln_param_table_find_by_arg_name(op->op_arg->params, "start")->value_int, 1);
     ck_assert_int_eq(ln_param_table_find_by_arg_name(op->op_arg->params, "len")->type,
                      LN_PARAM_NUMBER);
     ck_assert_int_eq(ln_param_table_find_by_arg_name(op->op_arg->params, "len")->value_int, 3);

     /* reshape1 */
     op = ln_list_nth_data(ops, 2);
     ck_assert_ptr_eq(op->pre_run, ln_op_list_find_by_optype(registered_ops, "reshape")->pre_run);
     ck_assert_ptr_eq(op->run, ln_op_list_find_by_optype(registered_ops, "reshape")->run);
     ck_assert_ptr_eq(op->post_run, ln_op_list_find_by_optype(registered_ops,
                                                              "reshape")->post_run);
     ck_assert_str_eq(op->op_arg->name, "reshape1");
     ck_assert_str_eq(op->op_arg->optype, "reshape");
     ck_assert_ptr_eq(op->op_arg->priv, NULL);
     ck_assert_str_eq(ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "src")->name,
                      "slice1");
     ck_assert_ptr_eq(ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "src")->tensor,
                      tensor1);
     ck_assert_str_eq(ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "dst")->name,
                      "reshape1");
     tensor1 = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "dst")->tensor;
     ck_assert_int_eq(ln_param_table_find_by_arg_name(op->op_arg->params, "dims")->type,
                      LN_PARAM_ARRAY_NUMBER);
     ck_assert_int_eq(ln_param_table_find_by_arg_name(op->op_arg->params,
                                                      "dims")->value_array_int[0], 3);
     ck_assert_int_eq(ln_param_table_find_by_arg_name(op->op_arg->params,
                                                      "dims")->value_array_int[1], 2);

     /* maxreduce1 */
     op = ln_list_nth_data(ops, 3);
     ck_assert_ptr_eq(op->pre_run, ln_op_list_find_by_optype(registered_ops,
                                                             "maxreduce")->pre_run);
     ck_assert_ptr_eq(op->run, ln_op_list_find_by_optype(registered_ops, "maxreduce")->run);
     ck_assert_ptr_eq(op->post_run, ln_op_list_find_by_optype(registered_ops,
                                                              "maxreduce")->post_run);
     ck_assert_str_eq(op->op_arg->name, "maxreduce1");
     ck_assert_str_eq(op->op_arg->optype, "maxreduce");
     ck_assert_ptr_eq(op->op_arg->priv, NULL);
     ck_assert_str_eq(ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "src")->name,
                      "reshape1");
     ck_assert_ptr_eq(ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "src")->tensor,
                      tensor1);
     ck_assert_str_eq(ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "dst")->name,
                      "maxreduce1_dst");
     tensor1 = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "dst")->tensor;
     ck_assert_str_eq(ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "arg")->name,
                      "maxreduce1_arg");
     tensor2 = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "arg")->tensor;
     ck_assert_int_eq(ln_param_table_find_by_arg_name(op->op_arg->params, "axis")->type,
                      LN_PARAM_NUMBER);
     ck_assert_int_eq(ln_param_table_find_by_arg_name(op->op_arg->params, "axis")->value_int, 0);

     /* elew1 */
     op = ln_list_nth_data(ops, 4);
     ck_assert_ptr_eq(op->pre_run, ln_op_list_find_by_optype(registered_ops, "elew")->pre_run);
     ck_assert_ptr_eq(op->run, ln_op_list_find_by_optype(registered_ops, "elew")->run);
     ck_assert_ptr_eq(op->post_run, ln_op_list_find_by_optype(registered_ops, "elew")->post_run);
     ck_assert_str_eq(op->op_arg->name, "elew1");
     ck_assert_str_eq(op->op_arg->optype, "elew");
     ck_assert_ptr_ne(op->op_arg->priv, NULL);
     ck_assert_str_eq(ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "src1")->name,
                      "maxreduce1_dst");
     ck_assert_ptr_eq(ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "src1")->tensor,
                      tensor1);
     ck_assert_str_eq(ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "src2")->name,
                      "maxreduce1_arg");
     ck_assert_ptr_eq(ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "src2")->tensor,
                      tensor2);
     ck_assert_str_eq(ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "dst")->name,
                      "elew1");
     tensor1 = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "dst")->tensor;
     ck_assert_int_eq(ln_param_table_find_by_arg_name(op->op_arg->params, "elew_op")->type,
                      LN_PARAM_STRING);
     ck_assert_str_eq(ln_param_table_find_by_arg_name(op->op_arg->params, "elew_op")->value_string,
                      "TL_MUL");

     /* transpose1 */
     op = ln_list_nth_data(ops, 5);
     ck_assert_ptr_eq(op->pre_run, ln_op_list_find_by_optype(registered_ops,
                                                             "transpose")->pre_run);
     ck_assert_ptr_eq(op->run, ln_op_list_find_by_optype(registered_ops, "transpose")->run);
     ck_assert_ptr_eq(op->post_run, ln_op_list_find_by_optype(registered_ops,
                                                              "transpose")->post_run);
     ck_assert_str_eq(op->op_arg->name, "transpose1");
     ck_assert_str_eq(op->op_arg->optype, "transpose");
     ck_assert_ptr_ne(op->op_arg->priv, NULL);
     ck_assert_str_eq(ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "src")->name,
                      "elew1");
     ck_assert_ptr_eq(ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "src")->tensor,
                      tensor1);
     ck_assert_str_eq(ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "dst")->name,
                      "transpose1");
     ck_assert_int_eq(ln_param_table_find_by_arg_name(op->op_arg->params, "axes")->type,
                      LN_PARAM_ARRAY_NUMBER);
     ck_assert_int_eq(ln_param_table_find_by_arg_name(op->op_arg->params,
                                                      "axes")->value_array_int[0], 1);
     ck_assert_int_eq(ln_param_table_find_by_arg_name(op->op_arg->params,
                                                      "axes")->value_array_int[1], 0);

     ln_op_list_do_post_run(ops, &error);
     ln_error_handle(&error);
     ln_op_list_free_tables_too(ops);
}
END_TEST
/* end of tests */

Suite *make_parse_suite(void)
{
     Suite *s;
     TCase *tc_parse;

     s = suite_create("parse");
     tc_parse = tcase_create("parse");
     tcase_add_checked_fixture(tc_parse, setup, teardown);

     tcase_add_test(tc_parse, test_ln_parse_ops);
     /* end of adding tests */

     suite_add_tcase(s, tc_parse);

     return s;
}

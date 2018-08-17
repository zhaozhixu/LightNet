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
     ln_op *op, *op_proto;
     ln_param_entry *param_entry;
     ln_tensor_entry *tensor_entry;
     tl_tensor *tensor1, *tensor2, *tensor_true;

     ops = ln_parse_ops(json_str, registered_ops, &error);

     /* create1 */
     op = ln_op_list_find_by_name(ops, "create1");
     op_proto = ln_op_list_find_by_optype(registered_ops, "create");
     ck_assert_ptr_eq(op->pre_run, op_proto->pre_run);
     ck_assert_ptr_eq(op->run, op_proto->run);
     ck_assert_ptr_eq(op->post_run, op_proto->post_run);
     ck_assert_str_eq(op->op_arg->name, "create1");
     ck_assert_str_eq(op->op_arg->optype, "create");
     ck_assert_ptr_ne(op->op_arg->priv, NULL);

     tensor_entry = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "dst");
     ck_assert_str_eq(tensor_entry->name, "create1");
     tensor_true = tl_tensor_create((float[]){1, 2, 3, 4, 5, 6, 7, 8},
                                    2, (int[]){2, 4}, TL_FLOAT);
     tl_assert_tensor_eq(tensor_true, tensor_entry->tensor);
     tl_tensor_free(tensor_true);
     tensor1 = tensor_entry->tensor;

     param_entry = ln_param_table_find_by_arg_name(op->op_arg->params, "dims");
     ck_assert_int_eq(param_entry->type, LN_PARAM_ARRAY_NUMBER);
     ck_assert_array_int_eq(param_entry->value_array_int, ck_array(int, 2, 4), 2);

     /* slice1 */
     op = ln_op_list_find_by_name(ops, "slice1");
     op_proto = ln_op_list_find_by_optype(registered_ops, "slice");
     ck_assert_ptr_eq(op->pre_run, op_proto->pre_run);
     ck_assert_ptr_eq(op->run, op_proto->run);
     ck_assert_ptr_eq(op->post_run, op_proto->post_run);
     ck_assert_str_eq(op->op_arg->name, "slice1");
     ck_assert_str_eq(op->op_arg->optype, "slice");
     ck_assert_ptr_ne(op->op_arg->priv, NULL);

     tensor_entry = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "src");
     ck_assert_str_eq(tensor_entry->name, "create1");
     ck_assert_ptr_eq(tensor_entry->tensor, tensor1);
     tensor_entry = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "dst");
     ck_assert_str_eq(tensor_entry->name, "slice1");
     tensor_true = tl_tensor_create((float[]){0, 0, 0, 0, 0, 0},
                                   2, (int[]){2, 3}, TL_FLOAT);
     tl_assert_tensor_eq(tensor_true, tensor_entry->tensor);
     tl_tensor_free(tensor_true);
     tensor1 = tensor_entry->tensor;

     param_entry = ln_param_table_find_by_arg_name(op->op_arg->params, "axis");
     ck_assert_int_eq(param_entry->type, LN_PARAM_NUMBER);
     ck_assert_int_eq(param_entry->value_int, 1);
     param_entry = ln_param_table_find_by_arg_name(op->op_arg->params, "start");
     ck_assert_int_eq(param_entry->type, LN_PARAM_NUMBER);
     ck_assert_int_eq(param_entry->value_int, 1);
     param_entry = ln_param_table_find_by_arg_name(op->op_arg->params, "len");
     ck_assert_int_eq(param_entry->type, LN_PARAM_NUMBER);
     ck_assert_int_eq(param_entry->value_int, 3);

     /* reshape1 */
     op = ln_op_list_find_by_name(ops, "reshape1");
     op_proto = ln_op_list_find_by_optype(registered_ops, "reshape");
     ck_assert_ptr_eq(op->pre_run, op_proto->pre_run);
     ck_assert_ptr_eq(op->run, op_proto->run);
     ck_assert_ptr_eq(op->post_run, op_proto->post_run);
     ck_assert_str_eq(op->op_arg->name, "reshape1");
     ck_assert_str_eq(op->op_arg->optype, "reshape");
     ck_assert_ptr_ne(op->op_arg->priv, NULL);

     tensor_entry = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "src");
     ck_assert_str_eq(tensor_entry->name, "slice1");
     ck_assert_ptr_eq(tensor_entry->tensor, tensor1);
     tensor_entry = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "dst");
     ck_assert_str_eq(tensor_entry->name, "reshape1");
     tensor_true = tl_tensor_create((float[]){0, 0, 0, 0, 0, 0},
                                   2, (int[]){3, 2}, TL_FLOAT);
     tl_assert_tensor_eq(tensor_true, tensor_entry->tensor);
     tl_tensor_free(tensor_true);

     tensor1 = tensor_entry->tensor;

     param_entry = ln_param_table_find_by_arg_name(op->op_arg->params, "dims");
     ck_assert_int_eq(param_entry->type, LN_PARAM_ARRAY_NUMBER);
     ck_assert_array_int_eq(param_entry->value_array_int, ck_array(int, 3, 2), 2);

     /* maxreduce1 */
     op = ln_op_list_find_by_name(ops, "maxreduce1");
     op_proto = ln_op_list_find_by_optype(registered_ops, "maxreduce");
     ck_assert_ptr_eq(op->pre_run, op_proto->pre_run);
     ck_assert_ptr_eq(op->run, op_proto->run);
     ck_assert_ptr_eq(op->post_run, op_proto->post_run);
     ck_assert_str_eq(op->op_arg->name, "maxreduce1");
     ck_assert_str_eq(op->op_arg->optype, "maxreduce");
     ck_assert_ptr_ne(op->op_arg->priv, NULL);

     tensor_entry = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "src");
     ck_assert_str_eq(tensor_entry->name, "reshape1");
     ck_assert_ptr_eq(tensor_entry->tensor, tensor1);
     tensor_entry = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "dst");
     ck_assert_str_eq(tensor_entry->name, "maxreduce1_dst");
     tensor_true = tl_tensor_create((float[]){0, 0},
                                   2, (int[]){1, 2}, TL_FLOAT);
     tl_assert_tensor_eq(tensor_true, tensor_entry->tensor);
     tl_tensor_free(tensor_true);
     tensor1 = tensor_entry->tensor;
     tensor_entry = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "arg");
     ck_assert_str_eq(tensor_entry->name, "maxreduce1_arg");
     tensor_true = tl_tensor_create((float[]){0, 0},
                                   2, (int[]){1, 2}, TL_FLOAT);
     tl_assert_tensor_eq(tensor_true, tensor_entry->tensor);
     tl_tensor_free(tensor_true);
     tensor2 = tensor_entry->tensor;

     param_entry = ln_param_table_find_by_arg_name(op->op_arg->params, "axis");
     ck_assert_int_eq(param_entry->type, LN_PARAM_NUMBER);
     ck_assert_int_eq(param_entry->value_int, 0);

     /* elew1 */
     op = ln_op_list_find_by_name(ops, "elew1");
     op_proto = ln_op_list_find_by_optype(registered_ops, "elew");
     ck_assert_ptr_eq(op->pre_run, op_proto->pre_run);
     ck_assert_ptr_eq(op->run, op_proto->run);
     ck_assert_ptr_eq(op->post_run, op_proto->post_run);
     ck_assert_str_eq(op->op_arg->name, "elew1");
     ck_assert_str_eq(op->op_arg->optype, "elew");
     ck_assert_ptr_ne(op->op_arg->priv, NULL);

     tensor_entry = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "src1");
     ck_assert_str_eq(tensor_entry->name, "maxreduce1_dst");
     ck_assert_ptr_eq(tensor_entry->tensor, tensor1);
     tensor_entry = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "src2");
     ck_assert_str_eq(tensor_entry->name, "maxreduce1_arg");
     ck_assert_ptr_eq(tensor_entry->tensor, tensor2);
     tensor_entry = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "dst");
     ck_assert_str_eq(tensor_entry->name, "elew1");
     tensor_true = tl_tensor_create((float[]){0, 0},
                                   2, (int[]){1, 2}, TL_FLOAT);
     tl_assert_tensor_eq(tensor_true, tensor_entry->tensor);
     tl_tensor_free(tensor_true);
     tensor1 = tensor_entry->tensor;

     param_entry = ln_param_table_find_by_arg_name(op->op_arg->params, "elew_op");
     ck_assert_int_eq(param_entry->type, LN_PARAM_STRING);
     ck_assert_str_eq(param_entry->value_string, "TL_MUL");

     /* transpose1 */
     op = ln_op_list_find_by_name(ops, "transpose1");
     op_proto = ln_op_list_find_by_optype(registered_ops, "transpose");
     ck_assert_ptr_eq(op->pre_run, op_proto->pre_run);
     ck_assert_ptr_eq(op->run, op_proto->run);
     ck_assert_ptr_eq(op->post_run, op_proto->post_run);
     ck_assert_str_eq(op->op_arg->name, "transpose1");
     ck_assert_str_eq(op->op_arg->optype, "transpose");
     ck_assert_ptr_ne(op->op_arg->priv, NULL);

     tensor_entry = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "src");
     ck_assert_str_eq(tensor_entry->name, "elew1");
     ck_assert_ptr_eq(tensor_entry->tensor, tensor1);
     tensor_entry = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "dst");
     ck_assert_str_eq(tensor_entry->name, "transpose1");
     tensor_true = tl_tensor_create((float[]){0, 0},
                                   2, (int[]){2, 1}, TL_FLOAT);
     tl_assert_tensor_eq(tensor_true, tensor_entry->tensor);
     tl_tensor_free(tensor_true);

     param_entry = ln_param_table_find_by_arg_name(op->op_arg->params, "axes");
     ck_assert_int_eq(param_entry->type, LN_PARAM_ARRAY_NUMBER);
     ck_assert_array_int_eq(param_entry->value_array_int, ck_array(int, 1, 0), 2);

     /* zeros1 */
     op = ln_op_list_find_by_name(ops, "zeros1");
     op_proto = ln_op_list_find_by_optype(registered_ops, "zeros");
     ck_assert_ptr_eq(op->pre_run,op_proto ->pre_run);
     ck_assert_ptr_eq(op->run, op_proto->run);
     ck_assert_ptr_eq(op->post_run, op_proto->post_run);
     ck_assert_str_eq(op->op_arg->name, "zeros1");
     ck_assert_str_eq(op->op_arg->optype, "zeros");
     ck_assert_ptr_ne(op->op_arg->priv, NULL);

     tensor_entry = ln_tensor_table_find_by_arg_name(op->op_arg->tensors, "dst");
     ck_assert_str_eq(tensor_entry->name, "zeros1");
     tensor_true = tl_tensor_create((float[]){0, 0, 0, 0, 0, 0, 0, 0},
                                   2, (int[]){2, 4}, TL_FLOAT);
     tl_assert_tensor_eq(tensor_true, tensor_entry->tensor);
     tl_tensor_free(tensor_true);

     param_entry = ln_param_table_find_by_arg_name(op->op_arg->params, "dtype");
     ck_assert_int_eq(param_entry->type, LN_PARAM_STRING);
     ck_assert_str_eq(param_entry->value_string, "TL_FLOAT");
     param_entry = ln_param_table_find_by_arg_name(op->op_arg->params, "dims");
     ck_assert_int_eq(param_entry->type, LN_PARAM_ARRAY_NUMBER);
     ck_assert_array_int_eq(param_entry->value_array_int, ck_array(int, 2, 4), 2);

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

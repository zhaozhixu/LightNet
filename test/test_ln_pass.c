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
#include "../src/ln_pass.h"

static char *json_str;
static ln_list *reg_ops;
static ln_error *error = NULL;

extern ln_op *ln_init_ops[];

static void setup(void)
{
     struct stat buf;
     FILE *fp;
     size_t n;

     if (stat("test_ln_pass.json", &buf) < 0) {
          perror("Cannot stat test_ln_pass.json");
          exit(EXIT_FAILURE);
     }

     json_str = ln_alloc(buf.st_size+1);
     if (!(fp = fopen("test_ln_pass.json", "rb"))) {
          perror("Cannot open test_ln_pass.json");
          exit(EXIT_FAILURE);
     }
     n = fread(json_str, buf.st_size, 1, fp);
     if (n < 1 && ferror(fp)) {
          perror("Error reading test_ln_pass.json");
          exit(EXIT_FAILURE);
     }

     fclose(fp);

     reg_ops = ln_op_list_create_from_array(ln_init_ops);
}

static void teardown(void)
{
     ln_free(json_str);
     ln_op_list_free(reg_ops);
}

static void assert_op_eq(ln_list *registered_ops, ln_op *op,
                         char *optype, char *opname)
{
     ln_op *op_proto;

     op_proto = ln_op_list_find_by_optype(registered_ops, optype);
     ck_assert_ptr_ne(op, NULL);
     ck_assert_ptr_eq(op->pre_run, op_proto->pre_run);
     ck_assert_ptr_eq(op->static_run, op_proto->static_run);
     ck_assert_ptr_eq(op->run, op_proto->run);
     ck_assert_ptr_eq(op->post_run, op_proto->post_run);
     ck_assert_str_eq(op->op_arg->optype, op_proto->op_arg->optype);
     ck_assert_int_eq(op->op_arg->mtype_in, op_proto->op_arg->mtype_in);
     ck_assert_int_eq(op->op_arg->mtype_out, op->op_arg->mtype_out);
     ck_assert_str_eq(op->op_arg->name, opname);
}

static void assert_tensor_eq(ln_list *tlist, ln_hash *table, char *arg_name,
                            char *name, tl_tensor *true_tensor,
                            tl_tensor **find_tensor)
{
     char *find_name;
     ln_tensor_entry *entry;

     find_name = ln_tensor_list_find_name(tlist, arg_name);
     ck_assert_str_eq(find_name, name);
     entry = ln_tensor_table_find(table, find_name);
     tl_assert_tensor_eq(true_tensor, entry->tensor);
     if (find_tensor)
          *find_tensor = entry->tensor;
}

#define TENSORS_OUT (op->op_arg->tensors_out)
#define TENSORS_IN (op->op_arg->tensors_in)
#define PARAMS (op->op_arg->params)
#define ARR(type, varg...) (type[]){varg}

START_TEST(test_ln_pass_parse)
{
     ln_list *ops;
     ln_op *op;
     ln_param_entry *param_entry;
     char *tensor_name;
     ln_hash *table;
     /* tl_tensor *tensor1, *tensor2, *tensor_true; */

     table = ln_tensor_table_create();
     ops = ln_pass_parse(json_str, reg_ops, table);

     /* create1 */
     op = ln_op_list_find_by_name(ops, "create1");
     assert_op_eq(reg_ops, op, "create", "create1");

     /* tensor_true = tl_tensor_create(ARR(float,1,2,3,4,5,6,7,8), 2, ARR(int,2,4), TL_FLOAT); */
     /* assert_tensor_eq(TENSORS_OUT, table, "dst", "create1", tensor_true, &tensor1); */
     /* tl_tensor_free(tensor_true); */
     tensor_name = ln_tensor_list_find_name(TENSORS_OUT, "dst");
     ck_assert_ptr_ne(tensor_name, NULL);
     ck_assert_str_eq(tensor_name, "create1");

     param_entry = ln_param_list_find(PARAMS, "dims");
     ck_assert_ptr_ne(param_entry, NULL);
     ck_assert_int_eq(param_entry->type, LN_PARAM_ARRAY_NUMBER);
     ck_assert_array_int_eq(param_entry->value_array_int, ARR(int,2,4), 2);

     /* slice1 */
     op = ln_op_list_find_by_name(ops, "slice1");
     assert_op_eq(reg_ops, op, "slice", "slice1");

     /* assert_tensor_eq(TENSORS_IN, table, "src", "create1", tensor1, NULL); */
     /* tensor_true = tl_tensor_create(ARR(float,0,0,0,0,0,0), 2, ARR(int,2,3), TL_FLOAT); */
     /* assert_tensor_eq(TENSORS_OUT, table, "dst", "slice1", tensor_true, &tensor1); */
     /* tl_tensor_free(tensor_true); */
     tensor_name = ln_tensor_list_find_name(TENSORS_IN, "src");
     ck_assert_ptr_ne(tensor_name, NULL);
     ck_assert_str_eq(tensor_name, "create1");
     tensor_name = ln_tensor_list_find_name(TENSORS_OUT, "dst");
     ck_assert_ptr_ne(tensor_name, NULL);
     ck_assert_str_eq(tensor_name, "slice1");

     param_entry = ln_param_list_find(PARAMS, "axis");
     ck_assert_ptr_ne(param_entry, NULL);
     ck_assert_int_eq(param_entry->type, LN_PARAM_NUMBER);
     ck_assert_int_eq(param_entry->value_int, 1);
     param_entry = ln_param_list_find(PARAMS, "start");
     ck_assert_ptr_ne(param_entry, NULL);
     ck_assert_int_eq(param_entry->type, LN_PARAM_NUMBER);
     ck_assert_int_eq(param_entry->value_int, 1);
     param_entry = ln_param_list_find(PARAMS, "len");
     ck_assert_ptr_ne(param_entry, NULL);
     ck_assert_int_eq(param_entry->type, LN_PARAM_NUMBER);
     ck_assert_int_eq(param_entry->value_int, 3);

     /* reshape1 */
     op = ln_op_list_find_by_name(ops, "reshape1");
     assert_op_eq(reg_ops, op, "reshape", "reshape1");

     /* assert_tensor_eq(TENSORS_IN, table, "src", "slice1", tensor1, NULL); */
     /* tensor_true = tl_tensor_create(ARR(float,0,0,0,0,0,0), 2, ARR(int,3,2), TL_FLOAT); */
     /* assert_tensor_eq(TENSORS_OUT, table, "dst", "reshape1", tensor_true, &tensor1); */
     /* tl_tensor_free(tensor_true); */
     tensor_name = ln_tensor_list_find_name(TENSORS_IN, "src");
     ck_assert_ptr_ne(tensor_name, NULL);
     ck_assert_str_eq(tensor_name, "slice1");
     tensor_name = ln_tensor_list_find_name(TENSORS_OUT, "dst");
     ck_assert_ptr_ne(tensor_name, NULL);
     ck_assert_str_eq(tensor_name, "reshape1");

     param_entry = ln_param_list_find(PARAMS, "dims");
     ck_assert_ptr_ne(param_entry, NULL);
     ck_assert_int_eq(param_entry->type, LN_PARAM_ARRAY_NUMBER);
     ck_assert_array_int_eq(param_entry->value_array_int, ARR(int,3,2), 2);

     /* maxreduce1 */
     op = ln_op_list_find_by_name(ops, "maxreduce1");
     assert_op_eq(reg_ops, op, "maxreduce", "maxreduce1");

     /* assert_tensor_eq(TENSORS_IN, table, "src", "reshape1", tensor1, NULL); */
     /* tensor_true = tl_tensor_create(ARR(float,0,0), 2, ARR(int,1,2), TL_FLOAT); */
     /* assert_tensor_eq(TENSORS_OUT, table, "dst", "maxreduce1_dst", tensor_true, &tensor1); */
     /* tl_tensor_free(tensor_true); */
     /* tensor_true = tl_tensor_create(ARR(float,0,0), 2, ARR(int,1,2), TL_FLOAT); */
     /* assert_tensor_eq(TENSORS_OUT, table, "arg", "maxreduce1_arg", tensor_true, &tensor2); */
     /* tl_tensor_free(tensor_true); */
     tensor_name = ln_tensor_list_find_name(TENSORS_IN, "src");
     ck_assert_ptr_ne(tensor_name, NULL);
     ck_assert_str_eq(tensor_name, "reshape1");
     tensor_name = ln_tensor_list_find_name(TENSORS_OUT, "dst");
     ck_assert_ptr_ne(tensor_name, NULL);
     ck_assert_str_eq(tensor_name, "maxreduce1_dst");
     tensor_name = ln_tensor_list_find_name(TENSORS_OUT, "arg");
     ck_assert_ptr_ne(tensor_name, NULL);
     ck_assert_str_eq(tensor_name, "maxreduce1_arg");

     param_entry = ln_param_list_find(op->op_arg->params, "axis");
     ck_assert_ptr_ne(param_entry, NULL);
     ck_assert_int_eq(param_entry->type, LN_PARAM_NUMBER);
     ck_assert_int_eq(param_entry->value_int, 0);

     /* elew1 */
     op = ln_op_list_find_by_name(ops, "elew1");
     assert_op_eq(reg_ops, op, "elew", "elew1");

     /* assert_tensor_eq(TENSORS_IN, table, "src1", "maxreduce1_dst", tensor1, NULL); */
     /* assert_tensor_eq(TENSORS_IN, table, "src2", "maxreduce1_arg", tensor1, NULL); */
     /* tensor_true = tl_tensor_create(ARR(float,0,0), 2, ARR(int,1,2), TL_FLOAT); */
     /* assert_tensor_eq(TENSORS_OUT, table, "dst", "elew1", tensor_true, &tensor1); */
     /* tl_tensor_free(tensor_true); */
     tensor_name = ln_tensor_list_find_name(TENSORS_IN, "src1");
     ck_assert_ptr_ne(tensor_name, NULL);
     ck_assert_str_eq(tensor_name, "maxreduce1_dst");
     tensor_name = ln_tensor_list_find_name(TENSORS_IN, "src2");
     ck_assert_ptr_ne(tensor_name, NULL);
     ck_assert_str_eq(tensor_name, "maxreduce1_arg");
     tensor_name = ln_tensor_list_find_name(TENSORS_OUT, "dst");
     ck_assert_ptr_ne(tensor_name, NULL);
     ck_assert_str_eq(tensor_name, "elew1");

     param_entry = ln_param_list_find(op->op_arg->params, "elew_op");
     ck_assert_ptr_ne(param_entry, NULL);
     ck_assert_int_eq(param_entry->type, LN_PARAM_STRING);
     ck_assert_str_eq(param_entry->value_string, "TL_MUL");

     /* transpose1 */
     op = ln_op_list_find_by_name(ops, "transpose1");
     assert_op_eq(reg_ops, op, "transpose", "transpose1");

     /* assert_tensor_eq(TENSORS_IN, table, "src", "elew1", tensor1, NULL); */
     /* tensor_true = tl_tensor_create(ARR(float,0,0), 2, ARR(int,2,1), TL_FLOAT); */
     /* assert_tensor_eq(TENSORS_OUT, table, "dst", "transpose1", tensor_true, NULL); */
     /* tl_tensor_free(tensor_true); */
     tensor_name = ln_tensor_list_find_name(TENSORS_IN, "src");
     ck_assert_ptr_ne(tensor_name, NULL);
     ck_assert_str_eq(tensor_name, "elew1");
     tensor_name = ln_tensor_list_find_name(TENSORS_OUT, "dst");
     ck_assert_ptr_ne(tensor_name, NULL);
     ck_assert_str_eq(tensor_name, "transpose1");

     param_entry = ln_param_list_find(op->op_arg->params, "axes");
     ck_assert_ptr_ne(param_entry, NULL);
     ck_assert_int_eq(param_entry->type, LN_PARAM_ARRAY_NUMBER);
     ck_assert_array_int_eq(param_entry->value_array_int, ARR(int,1,0), 2);

     /* zeros1 */
     op = ln_op_list_find_by_name(ops, "zeros1");
     assert_op_eq(reg_ops, op, "zeros", "zeros1");

     /* tensor_true = tl_tensor_create(ARR(float,0,0,0,0,0,0,0,0), 2, ARR(int,2,4), TL_FLOAT); */
     /* assert_tensor_eq(TENSORS_OUT, table, "dst", "zeros1", tensor_true, NULL); */
     /* tl_tensor_free(tensor_true); */
     tensor_name = ln_tensor_list_find_name(TENSORS_OUT, "dst");
     ck_assert_ptr_ne(tensor_name, NULL);
     ck_assert_str_eq(tensor_name, "zeros1");


     param_entry = ln_param_list_find(op->op_arg->params, "dtype");
     ck_assert_ptr_ne(param_entry, NULL);
     ck_assert_int_eq(param_entry->type, LN_PARAM_STRING);
     ck_assert_str_eq(param_entry->value_string, "TL_FLOAT");
     param_entry = ln_param_list_find(op->op_arg->params, "dims");
     ck_assert_ptr_ne(param_entry, NULL);
     ck_assert_int_eq(param_entry->type, LN_PARAM_ARRAY_NUMBER);
     ck_assert_array_int_eq(param_entry->value_array_int, ARR(int,2,4), 2);

     ln_op_list_do_post_run(ops, &error);
     ln_error_handle(&error);
     ln_op_list_free_lists_too(ops);
     ln_tensor_table_free(table);
}
END_TEST

static void mem_pool_free_wrapper(void *p)
{
     ln_mem_pool_free(p);
}

START_TEST(test_ln_pass_mem)
{
     ln_hash *table;
     ln_list *ops;
     ln_mem_pool *mem_pool_cpu;
     ln_hash *mem_pools;
     ln_error *error = NULL;

     mem_pool_cpu = ln_mem_pool_create(4096, 1);
     mem_pools = ln_hash_create(ln_direct_hash, ln_direct_cmp, NULL, mem_pool_free_wrapper);
     ln_hash_insert(mem_pools, (void *)LN_MEM_CPU, mem_pool_cpu);
     table = ln_tensor_table_create();
     ops = ln_pass_parse(json_str, reg_ops, table);

     ops = ln_pass_mem(ops, mem_pools);

     ln_op_list_do_post_run(ops, &error);
     ln_error_handle(&error);
     ln_op_list_free_lists_too(ops);
     ln_hash_free(mem_pools);
     ln_tensor_table_free(table);
}
END_TEST
/* end of tests */

Suite *make_pass_suite(void)
{
     Suite *s;
     TCase *tc_pass;

     s = suite_create("pass");
     tc_pass = tcase_create("pass");
     tcase_add_checked_fixture(tc_pass, setup, teardown);

     tcase_add_test(tc_pass, test_ln_pass_parse);
     tcase_add_test(tc_pass, test_ln_pass_mem);
     /* end of adding tests */

     suite_add_tcase(s, tc_pass);

     return s;
}

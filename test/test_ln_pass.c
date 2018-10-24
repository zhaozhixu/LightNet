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
#include "../src/ln_pass.h"
#include "../src/ln_parse.h"
#include "../src/ln_arch.h"

extern ln_arch *ln_archs[];

static ln_list *reg_ops;
static ln_list *ops;
static char *json_str;
static ln_hash *tensor_table;
static ln_error *error = NULL;

static void checked_setup(void)
{
     reg_ops = ln_arch_create_oplist(ln_archs);
     json_str = ln_read_text("test_ops.json");
     tensor_table = ln_tensor_table_create();
     ops = ln_parse(json_str, reg_ops, tensor_table);
}

static void checked_teardown(void)
{
     ln_op_list_free(reg_ops);
     ln_free(json_str);
     ln_tensor_table_free(tensor_table);
     ln_op_list_free_lists_too(ops);
}

static void mem_pools_free_wrapper(void *p)
{
     ln_mem_pool_free(p);
}

START_TEST(test_ln_pass_mem)
{
     ln_hash *mem_pools;
     ln_mem_pool *mem_pool_cpu;
     ln_tensor_entry *te;

     mem_pools = ln_hash_create(ln_direct_hash , ln_direct_cmp, NULL, mem_pools_free_wrapper);
     mem_pool_cpu = ln_mem_pool_create(4096, 1);
     ln_hash_insert(mem_pools, (void *)LN_MEM_CPU, mem_pool_cpu);

     ln_op_list_do_pre_run(ops, &error);
     ln_error_handle(&error);
     ops = ln_pass_mem(ops, mem_pools);

     te = ln_tensor_table_find(tensor_table, "create1");
     ck_assert_int_eq(te->offset, 1);
     te = ln_tensor_table_find(tensor_table, "slice1");
     ck_assert_int_eq(te->offset, 33);
     te = ln_tensor_table_find(tensor_table, "reshape1");
     ck_assert_int_eq(te->offset, 0);
     te = ln_tensor_table_find(tensor_table, "maxreduce1_dst");
     ck_assert_int_eq(te->offset, 57);
     te = ln_tensor_table_find(tensor_table, "maxreduce1_arg");
     ck_assert_int_eq(te->offset, 65);
     te = ln_tensor_table_find(tensor_table, "elew1");
     ck_assert_int_eq(te->offset, 33);
     te = ln_tensor_table_find(tensor_table, "transpose1");
     ck_assert_int_eq(te->offset, 41);
     te = ln_tensor_table_find(tensor_table, "zeros1");
     ck_assert_int_eq(te->offset, 33);

     ln_hash_free(mem_pools);
}
END_TEST

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

#define TENSORS_OUT (op->op_arg->tensors_out)
#define TENSORS_IN (op->op_arg->tensors_in)
#define PARAMS (op->op_arg->params)
#define ARR(type, varg...) (type[]){varg}

/* TODO: test it throughly */
START_TEST(test_ln_pass_peephole)
{
     ln_op *op;
     ln_param_entry *param_entry;
     char *tensor_name;

     ln_op_list_do_pre_run(ops, &error);
     ops = ln_pass_peephole(ops, 1, ln_archs[1]->ph_funcs);

     /* create1 */
     op = ln_op_list_find_by_name(ops, "create1");
     assert_op_eq(reg_ops, op, "create_cuda", "create1");

     tensor_name = ln_tensor_list_find_name(TENSORS_OUT, "dst");
     ck_assert_ptr_ne(tensor_name, NULL);
     ck_assert_str_eq(tensor_name, "create1");

     param_entry = ln_param_list_find(PARAMS, "dims");
     ck_assert_ptr_ne(param_entry, NULL);
     ck_assert_int_eq(param_entry->type, LN_PARAM_ARRAY_NUMBER);
     ck_assert_array_int_eq(param_entry->value_array_int, ARR(int,2,4), 2);

     /* slice1 */
     op = ln_op_list_find_by_name(ops, "slice1");
     assert_op_eq(reg_ops, op, "slice_cuda", "slice1");

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
     assert_op_eq(reg_ops, op, "reshape_cuda", "reshape1");

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
     assert_op_eq(reg_ops, op, "maxreduce_cuda", "maxreduce1");

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
     assert_op_eq(reg_ops, op, "elew_cuda", "elew1");

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
     assert_op_eq(reg_ops, op, "transpose_cuda", "transpose1");

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
     assert_op_eq(reg_ops, op, "zeros_cuda", "zeros1");

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
}
END_TEST
/* end of tests */

Suite *make_pass_suite(void)
{
     Suite *s;
     TCase *tc_pass;

     s = suite_create("pass");
     tc_pass = tcase_create("pass");
     tcase_add_checked_fixture(tc_pass, checked_setup, checked_teardown);

     tcase_add_test(tc_pass, test_ln_pass_mem);
     tcase_add_test(tc_pass, test_ln_pass_peephole);
     /* end of adding tests */

     suite_add_tcase(s, tc_pass);

     return s;
}

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

extern ln_op *ln_init_ops[];
static ln_list *reg_ops;
static ln_list *ops;
static char *json_str;
static ln_hash *tensor_table;
static ln_error *error = NULL;

static void checked_setup(void)
{
     reg_ops = ln_op_list_create_from_array(ln_init_ops);
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
     ln_hash_insert(mem_pools, LN_MEM_CPU, mem_pool_cpu);

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
/* end of tests */

Suite *make_pass_suite(void)
{
     Suite *s;
     TCase *tc_pass;

     s = suite_create("pass");
     tc_pass = tcase_create("pass");
     tcase_add_checked_fixture(tc_pass, checked_setup, checked_teardown);

     tcase_add_test(tc_pass, test_ln_pass_mem);
     /* end of adding tests */

     suite_add_tcase(s, tc_pass);

     return s;
}

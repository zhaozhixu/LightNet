/*
 * Copyright (c) 2018-2020 Zhixu Zhao
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

#include <check.h>
#include <tl_check.h>
#include "lightnettest/ln_test.h"
#include "ln_pass.h"
#include "ln_json.h"
#include "ln_arch.h"
#include "ln_context.h"

static char *json_str;
static ln_context *ctx;

static void checked_setup(void)
{
    ln_arch_init();
    ctx = ln_context_create();
    json_str = ln_read_text(LN_TEST_DIR"/data/test_ops.json");
    ln_json_parse(json_str, ctx);
    ln_context_init_ops(ctx);
}

static void checked_teardown(void)
{
    ln_arch_cleanup();
    ln_context_cleanup_ops(ctx);
    ln_context_free(ctx);
    ln_free(json_str);
}

static void assert_op_eq(ln_op *op, char *optype, char *opname)
{
    ln_op *op_proto;

    op_proto = ln_hash_find(LN_ARCH.op_proto_table, optype);
    ck_assert_ptr_ne(op_proto, NULL);
    ck_assert_ptr_ne(op, NULL);
    ck_assert_str_eq(op->op_arg->optype, op_proto->op_arg->optype);
    ck_assert_str_eq(op->op_arg->name, opname);
    ck_assert_ptr_eq(op->pre_run, op_proto->pre_run);
    ck_assert_ptr_eq(op->static_run, op_proto->static_run);
    ck_assert_ptr_eq(op->run, op_proto->run);
    ck_assert_ptr_eq(op->post_run, op_proto->post_run);
}

#define TENSORS_OUT (op->op_arg->tensors_out)
#define TENSORS_IN (op->op_arg->tensors_in)
#define PARAMS (op->op_arg->params)
#define ARR(type, varg...) (type[]){varg}

static inline int can_replace(const char *optype)
{
    if (ln_streq(optype, "create") ||
        ln_streq(optype, "conv2d") ||
        ln_streq(optype, "maxpool2d") ||
        ln_streq(optype, "maxreduce") ||
        ln_streq(optype, "maxreduce_arg") ||
        ln_streq(optype, "relu") ||
        ln_streq(optype, "sigmoid") ||
        ln_streq(optype, "reshape") ||
        ln_streq(optype, "slice") ||
        ln_streq(optype, "transpose") ||
        ln_streq(optype, "zeros") ||
        ln_streq(optype, "elew") ||
        ln_streq(optype, "softmax") ||
        ln_streq(optype, "concat") ||
        ln_streq(optype, "upsample") ||
        ln_streq(optype, "sort1d") ||
        ln_streq(optype, "sort1d_by_key") ||
        ln_streq(optype, "arange") ||
        ln_streq(optype, "rearange") ||
        ln_streq(optype, "transform_bboxSQD") ||
        ln_streq(optype, "pick1d") ||
        ln_streq(optype, "lrelu") ||
        ln_streq(optype, "detect_yolov3") ||
        ln_streq(optype, "print"))
        return 1;
    return 0;

    /* for (int i = 0; ops_cuda[i]; i++) { */
    /*     if (ln_streq(ops_cuda[i]->op_arg->optype, optype)) */
    /*         return 1; */
    /* } */
    /* return 0; */
}

static ln_list *cb_func_single_replace(const ln_context *ctx,
                                       const ln_list *ops, size_t size,
                                       int *match)
{
    ln_op *op, *new_op, *op_proto;
    ln_op_arg *op_arg;
    ln_list *new_ops;
    char *optype_cuda;
    int *replace_flag;
    size_t i;

    *match = 0;
    replace_flag = ln_alloc(sizeof(int) * size);
    i = 0;
    LN_LIST_FOREACH(op, ops) {
        if (i >= size)
            break;
        if (can_replace(op->op_arg->optype)) {
            replace_flag[i++] = 1;
            *match = 1;
            continue;
        }
        replace_flag[i++] = 0;
    }
    if (!*match) {
        ln_free(replace_flag);
        return NULL;
    }

    new_ops = NULL;
    i = 0;
    LN_LIST_FOREACH(op, ops) {
        if (i >= size)
            break;
        op_arg = op->op_arg;
        if (!replace_flag[i++]) {
            new_op = ln_op_copy(op);
        } else {
            optype_cuda = ln_alloc(sizeof(char)*(strlen(op_arg->optype)+10));
            strcpy(optype_cuda, op_arg->optype);
            strcat(optype_cuda, "_cuda");
            ln_arch *arch = ln_hash_find(LN_ARCH.arch_table, "cuda");
            op_proto = ln_op_array_find_by_optype(arch->reg_ops, optype_cuda);
            assert(op_proto && "optype_cuda not found");
            new_op = ln_op_create_from_proto(op_proto, op_arg->name,
                                             ln_tensor_list_copy(op_arg->tensors_in),
                                             ln_tensor_list_copy(op_arg->tensors_out),
                                             ln_param_list_copy(op_arg->params),
                                             op_arg->tensor_table);
            ln_free(optype_cuda);
        }
        new_ops = ln_list_append(new_ops, new_op);
    }

    ln_free(replace_flag);
    return new_ops;
}

/* TODO: test it throughly */
LN_TEST_START(test_ln_pass_combiner)
{
#ifdef LN_CUDA
    ln_op *op;
    ln_param_entry *param_entry;
    char *tensor_name;

    ln_pass_combiner(ctx, 3, cb_func_single_replace);

    /* create1 */
    op = ln_op_list_find_by_name(ctx->ops, "create1");
    assert_op_eq(op, "create_cuda", "create1");

    tensor_name = ln_tensor_list_find_name(TENSORS_OUT, "dst");
    ck_assert_ptr_ne(tensor_name, NULL);
    ck_assert_str_eq(tensor_name, "create1");

    param_entry = ln_param_list_find(PARAMS, "dims");
    ck_assert_ptr_ne(param_entry, NULL);
    ck_assert_int_eq(param_entry->type, LN_PARAM_ARRAY_NUMBER);
    ck_assert_array_int_eq(param_entry->value_array_int, ARR(int,2,4), 2);

    /* slice1 */
    op = ln_op_list_find_by_name(ctx->ops, "slice1");
    assert_op_eq(op, "slice_cuda", "slice1");

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
    op = ln_op_list_find_by_name(ctx->ops, "reshape1");
    assert_op_eq(op, "reshape_cuda", "reshape1");

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
    op = ln_op_list_find_by_name(ctx->ops, "maxreduce1");
    assert_op_eq(op, "maxreduce_cuda", "maxreduce1");

    tensor_name = ln_tensor_list_find_name(TENSORS_IN, "src");
    ck_assert_ptr_ne(tensor_name, NULL);
    ck_assert_str_eq(tensor_name, "reshape1");
    tensor_name = ln_tensor_list_find_name(TENSORS_OUT, "dst");
    ck_assert_ptr_ne(tensor_name, NULL);
    ck_assert_str_eq(tensor_name, "maxreduce1_dst");

    param_entry = ln_param_list_find(op->op_arg->params, "axis");
    ck_assert_ptr_ne(param_entry, NULL);
    ck_assert_int_eq(param_entry->type, LN_PARAM_NUMBER);
    ck_assert_int_eq(param_entry->value_int, 0);

    /* elew1 */
    op = ln_op_list_find_by_name(ctx->ops, "elew1");
    assert_op_eq(op, "elew_cuda", "elew1");

    tensor_name = ln_tensor_list_find_name(TENSORS_IN, "src1");
    ck_assert_ptr_ne(tensor_name, NULL);
    ck_assert_str_eq(tensor_name, "maxreduce1_dst");
    tensor_name = ln_tensor_list_find_name(TENSORS_IN, "src2");
    ck_assert_ptr_ne(tensor_name, NULL);
    ck_assert_str_eq(tensor_name, "maxreduce1_dst");
    tensor_name = ln_tensor_list_find_name(TENSORS_OUT, "dst");
    ck_assert_ptr_ne(tensor_name, NULL);
    ck_assert_str_eq(tensor_name, "elew1");

    param_entry = ln_param_list_find(op->op_arg->params, "elew_op");
    ck_assert_ptr_ne(param_entry, NULL);
    ck_assert_int_eq(param_entry->type, LN_PARAM_STRING);
    ck_assert_str_eq(param_entry->value_string, "TL_MUL");

    /* transpose1 */
    op = ln_op_list_find_by_name(ctx->ops, "transpose1");
    assert_op_eq(op, "transpose_cuda", "transpose1");

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
    op = ln_op_list_find_by_name(ctx->ops, "zeros1");
    assert_op_eq(op, "zeros_cuda", "zeros1");

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
#endif
}
LN_TEST_END

static void mem_pools_free_wrapper(void *p)
{
    ln_mem_pool_free(p);
}

LN_TEST_START(test_ln_pass_mem)
{
#ifdef LN_CUDA
    ln_hash *mem_pools;
    ln_mem_pool *mem_pool_none;
    ln_tensor_entry *te;

    ln_pass_combiner(ctx, 3, cb_func_single_replace);
    mem_pools = ln_hash_create(ln_direct_hash, ln_direct_cmp, NULL, mem_pools_free_wrapper);
    mem_pool_none = ln_mem_pool_create(4096, 1);
    ln_hash_insert(mem_pools, (void *)LN_MEM_NONE, mem_pool_none);

    ln_pass_mem_plan(ctx);

    te = ln_tensor_table_find(ctx->tensor_table, "create1");
    ck_assert_ptr_ne(te, NULL);
    ck_assert_int_eq(te->offset, 32);
    te = ln_tensor_table_find(ctx->tensor_table, "slice1");
    ck_assert_ptr_ne(te, NULL);
    ck_assert_int_eq(te->offset, 64);
    te = ln_tensor_table_find(ctx->tensor_table, "reshape1");
    ck_assert_ptr_ne(te, NULL);
    ck_assert_int_eq(te->offset, 64);
    te = ln_tensor_table_find(ctx->tensor_table, "maxreduce1_dst");
    ck_assert_ptr_ne(te, NULL);
    ck_assert_int_eq(te->offset, 96);
    te = ln_tensor_table_find(ctx->tensor_table, "elew1");
    ck_assert_ptr_ne(te, NULL);
    ck_assert_int_eq(te->offset, 64);
    te = ln_tensor_table_find(ctx->tensor_table, "transpose1");
    ck_assert_ptr_ne(te, NULL);
    ck_assert_int_eq(te->offset, 96);
    te = ln_tensor_table_find(ctx->tensor_table, "zeros1");
    ck_assert_ptr_ne(te, NULL);
    ck_assert_int_eq(te->offset, 64);

    ln_hash_free(mem_pools);
#endif
}
LN_TEST_END

LN_TEST_TCASE_START(pass, checked_setup, checked_teardown)
{
    LN_TEST_ADD_TEST(test_ln_pass_combiner);
    LN_TEST_ADD_TEST(test_ln_pass_mem);
}
LN_TEST_TCASE_END

LN_TEST_ADD_TCASE(pass);

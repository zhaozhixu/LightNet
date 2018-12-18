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

#ifndef _LN_OP_H_
#define _LN_OP_H_

#include <math.h>
#include "ln_mem.h"
#include "ln_tensor.h"
#include "ln_param.h"
#include "ln_msg.h"
#include "ln_graph.h"

struct ln_op_arg {
    char            *name;
    char            *optype;
    char            *arch;
    ln_list         *tensors_in;
    ln_list         *tensors_out;
    ln_list         *params;
    ln_hash         *tensor_table;   /* TODO: goes outside ln_op? */
    void            *priv;           /* for other private data storage */
    const char     **in_arg_names;   /* NULL terminated array, as belows */
    const char     **out_arg_names;
    const char     **param_arg_names;
};
typedef struct ln_op_arg ln_op_arg;

typedef void (*ln_op_func) (ln_op_arg *op_arg);

/* The operator used in IR. */
/* NOTE: It is owned by a op_table. Remove it from a DFG and do post_run()
   before remove it from the op_table. */
struct ln_op {
    ln_op_arg   *op_arg;
    ln_op_func   pre_run;
    ln_op_func   static_run;
    ln_op_func   run;
    ln_op_func   post_run;
};
typedef struct ln_op ln_op;

#ifdef __cplusplus
LN_CPPSTART
#endif

ln_op *ln_op_create_from_proto(const ln_op *op_proto, const char *name,
                               ln_list *tensors_in, ln_list *tensors_out,
                               ln_list *params, ln_hash *tensor_table);
void ln_op_free(ln_op *op);
/* create with tensors_in, tensors_out, and with auto-generated unique op name
   and tensor names */
ln_op *ln_op_create_with_names(const ln_op *op_proto, ln_hash *tensor_table);
/* create with tensors_in, tensors_out, and with auto-generated unique op name */
ln_op *ln_op_create_with_opname(const ln_op *op_proto, ln_hash *tensor_table);
ln_op *ln_op_copy(const ln_op *op);
ln_op *ln_op_copy_to_optype(ln_hash *op_proto_table, const ln_op *op,
                            const char *new_optype);
ln_tensor_entry *ln_op_find_tensor_entry(const ln_op *op, const char *arg_name);
ln_tensor_list_entry *ln_op_find_tensor_list_entry(const ln_op *op,
                                                   const char *arg_name);

void ln_op_free_lists_too(ln_op *op);
ln_list *ln_op_list_create_from_array(ln_op **op_array);
void ln_op_list_free(ln_list *op_list);
void ln_op_list_free_lists_too(ln_list *ops);
ln_op *ln_op_list_find_by_optype(ln_list *ops, const char *optype);
ln_op *ln_op_array_find_by_optype(ln_op *ops[], const char *optype);
ln_op *ln_op_list_find_by_name(ln_list *ops, const char *name);
void ln_op_list_do_pre_run(ln_list *ops);
void ln_op_list_do_static_run(ln_list *ops);
void ln_op_list_do_run(ln_list *ops);
void ln_op_list_do_post_run(ln_list *ops);
/* Create a new opname with `prefix` subfixed with the next number.
   Need to be freed. `ops` should not be modified */
char *ln_op_list_new_opname(const ln_list *ops, const char *prefix);

ln_hash *ln_op_table_create(void);
int ln_op_table_insert(ln_hash *table, ln_op *op);
int ln_op_table_remove(ln_hash *table, const char *name);
ln_op *ln_op_table_find(ln_hash *table, const char *name);
void ln_op_table_free(ln_hash *table);

#ifdef __cplusplus
LN_CPPEND
#endif

/*
 * Convenient macros for operator parameter checking in a ln_op_func,
 * usually in a ln_op->pre_run function.
 * *level* is an enum defined in ln_msg.h
 * NOTE: If there is more error handling work, please write the code yourself
 * instead of using those macros.
 * NOTE: Normally we shouldn't use those kind of error handling routines in
 * ln_op->run and ln_op->post_run functions, where errors should be considered
 * as bugs.
 */
#define ln_opck(level, condition, msg_fmt, varg...)     \
    do {                                                \
        if (!(condition))                               \
            ln_msg_emit(level, msg_fmt, ##varg);        \
    } while (0)

#define ln_opck_satisfy_msg(condition, msg_fmt, varg...)        \
    ln_opck(LN_ERROR, (condition),                              \
            "%s(%s) should satisfy: "#msg_fmt,                  \
            op_arg->name, op_arg->optype, ##varg)

/* condition is appended as the message */
#define ln_opck_satisfy(condition)                      \
    ln_opck_satisfy_msg((condition), #condition)

/* entry should be returned by
   ln_param_list_find(op_arg->params, arg_name) */
#define ln_opck_param_exist(entry, arg_name)            \
    ln_opck(LN_ERROR, (entry),                          \
            "%s: `%s` needs a `%s` param",		\
            op_arg->optype, op_arg->name, (arg_name))

/*
 * entry should have been checked with ln_opck_param_exist
 * type is an enum defined in ln_param.h
 */
#define ln_opck_param_type(entry, param_type)                           \
    ln_opck(LN_ERROR, (entry)->type == (param_type),                    \
            "%s: `%s`'s `%s` param's value should be of type %s, but gets a %s", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            ln_param_type_name((param_type)), ln_param_type_name((entry)->type))

/* entry should be a LN_PARAM_NUMBER */
#define ln_opck_param_int_eq(entry, expect)                             \
    ln_opck(LN_ERROR, (entry)->value_int == (expect),                   \
            "%s: `%s`'s `%s` param's value should be == %d, but gets a %d", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect), (entry)->value_int)

/* entry should be a LN_PARAM_NUMBER */
#define ln_opck_param_int_gt(entry, expect)                             \
    ln_opck(LN_ERROR, (entry)->value_int > (expect),                    \
            "%s: `%s`'s `%s` param's value should be > %d, but gets a %d", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect), (entry)->value_int)

/* entry should be a LN_PARAM_NUMBER */
#define ln_opck_param_int_ge(entry, expect)                             \
    ln_opck(LN_ERROR, (entry)->value_int >= (expect),                   \
            "%s: `%s`'s `%s` param's value should be >= %d, but gets a %d", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect), (entry)->value_int)

/* entry should be a LN_PARAM_NUMBER */
#define ln_opck_param_int_lt(entry, expect)                             \
    ln_opck(LN_ERROR, (entry)->value_int < (expect),                    \
            "%s: `%s`'s `%s` param's value should be < %d, but gets a %d", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect), (entry)->value_int)

/* entry should be a LN_PARAM_NUMBER */
#define ln_opck_param_int_le(entry, expect)                             \
    ln_opck(LN_ERROR, (entry)->value_int <= (expect),                   \
            "%s: `%s`'s `%s` param's value should be <= %d, but gets a %d", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect), (entry)->value_int)

/* entry should be a LN_PARAM_NUMBER */
#define ln_opck_param_int_ne(entry, expect)                             \
    ln_opck(LN_ERROR, (entry)->value_int != (expect),                   \
            "%s: `%s`'s `%s` param's value should be != %d, but gets a %d", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect), (entry)->value_int)

/* entry should be a LN_PARAM_NUMBER */
#define ln_opck_param_float_eq(entry, expect)                           \
    ln_opck(LN_ERROR, (entry)->value_float == (expect),                 \
            "%s: `%s`'s `%s` param's value should be == %f, but gets a %f", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect), (entry)->value_float)

/* entry should be a LN_PARAM_NUMBER */
#define ln_opck_param_float_gt(entry, expect)                           \
    ln_opck(LN_ERROR, (entry)->value_float > (expect),                  \
            "%s: `%s`'s `%s` param's value should be > %f, but gets a %f", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect), (entry)->value_float)

/* entry should be a LN_PARAM_NUMBER */
#define ln_opck_param_float_ge(entry, expect)                           \
    ln_opck(LN_ERROR, (entry)->value_float >= (expect),                 \
            "%s: `%s`'s `%s` param's value should be >= %f, but gets a %f", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect), (entry)->value_float)

/* entry should be a LN_PARAM_NUMBER */
#define ln_opck_param_float_lt(entry, expect)                           \
    ln_opck(LN_ERROR, (entry)->value_float < (expect),                  \
            "%s: `%s`'s `%s` param's value should be < %f, but gets a %f", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect), (entry)->value_float)

/* entry should be a LN_PARAM_NUMBER */
#define ln_opck_param_float_le(entry, expect)                           \
    ln_opck(LN_ERROR, (entry)->value_float <= (expect),                 \
            "%s: `%s`'s `%s` param's value should be <= %f, but gets a %f", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect), (entry)->value_float)

/* entry should be a LN_PARAM_NUMBER */
#define ln_opck_param_float_ne(entry, expect)                           \
    ln_opck(LN_ERROR, (entry)->value_float != (expect),                 \
            "%s: `%s`'s `%s` param's value should be != %f, but gets a %f", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect), (entry)->value_float)

/* entry should be a LN_PARAM_NUMBER */
#define ln_opck_param_double_eq(entry, expect)                          \
    ln_opck(LN_ERROR, (entry)->value_double == (expect),                \
            "%s: `%s`'s `%s` param's value should be == %f, but gets a %f", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect), (entry)->value_double)

/* entry should be a LN_PARAM_NUMBER */
#define ln_opck_param_double_gt(entry, expect)                          \
    ln_opck(LN_ERROR, (entry)->value_double > (expect),                 \
            "%s: `%s`'s `%s` param's value should be > %f, but gets a %f", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect), (entry)->value_double)

/* entry should be a LN_PARAM_NUMBER */
#define ln_opck_param_double_ge(entry, expect)                          \
    ln_opck(LN_ERROR, (entry)->value_double >= (expect),                \
            "%s: `%s`'s `%s` param's value should be >= %f, but gets a %f", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect), (entry)->value_double)

/* entry should be a LN_PARAM_NUMBER */
#define ln_opck_param_double_lt(entry, expect)                          \
    ln_opck(LN_ERROR, (entry)->value_double < (expect),                 \
            "%s: `%s`'s `%s` param's value should be < %f, but gets a %f", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect), (entry)->value_double)

/* entry should be a LN_PARAM_NUMBER */
#define ln_opck_param_double_le(entry, expect)                          \
    ln_opck(LN_ERROR, (entry)->value_double <= (expect),                \
            "%s: `%s`'s `%s` param's value should be <= %f, but gets a %f", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect), (entry)->value_double)

/* entry should be a LN_PARAM_NUMBER */
#define ln_opck_param_double_ne(entry, expect)                          \
    ln_opck(LN_ERROR, (entry)->value_double != (expect),                \
            "%s: `%s`'s `%s` param's value should be != %f, but gets a %f", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect), (entry)->value_double)

/* entry should have been checked with ln_opck_param_exist */
#define ln_opck_param_array_len_eq(entry, expect_len)                   \
    ln_opck(LN_ERROR, (entry)->array_len == (expect_len),               \
            "%s: `%s`'s `%s` param needs %d elements, but gets %d elements", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect_len), (entry)->array_len)

#define ln_opck_param_array_len_gt(entry, expect_len)                   \
    ln_opck(LN_ERROR, (entry)->array_len > (expect_len),                \
            "%s: `%s`'s `%s` param needs > %d elements, but gets %d elements", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect_len), (entry)->array_len)

#define ln_opck_param_array_len_ge(entry, expect_len)                   \
    ln_opck(LN_ERROR, (entry)->array_len >= (expect_len),               \
            "%s: `%s`'s `%s` param needs >= %d elements, but gets %d elements", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect_len), (entry)->array_len)

#define ln_opck_param_array_len_lt(entry, expect_len)                   \
    ln_opck(LN_ERROR, (entry)->array_len < (expect_len),                \
            "%s: `%s`'s `%s` param needs < %d elements, but gets %d elements", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect_len), (entry)->array_len)

#define ln_opck_param_array_len_le(entry, expect_len)                   \
    ln_opck(LN_ERROR, (entry)->array_len <= (expect_len),               \
            "%s: `%s`'s `%s` param needs <= %d elements, but gets %d elements", \
            op_arg->optype, op_arg->name, (entry)->arg_name,            \
            (expect_len), (entry)->array_len)

#define ln_opck_param_array_int_eq(entry, expect)                       \
    do {                                                                \
        for (int i = 0; i < (entry)->array_len; i++)                    \
            ln_opck(LN_ERROR, (entry)->value_array_int[i] == (expect),  \
                    "%s: `%s`'s `%s` param[%d] should be == %d, but gets %d ", \
                    op_arg->optype, op_arg->name, (entry)->arg_name, i, \
                    (expect), (entry)->value_array_int[i]);             \
    } while (0)

#define ln_opck_param_array_int_gt(entry, expect)                       \
    do {                                                                \
        for (int i = 0; i < (entry)->array_len; i++)                    \
            ln_opck(LN_ERROR, (entry)->value_array_int[i] > (expect),   \
                    "%s: `%s`'s `%s` param[%d] should be > %d, but gets %d ", \
                    op_arg->optype, op_arg->name, (entry)->arg_name, i, \
                    (expect), (entry)->value_array_int[i]);             \
    } while (0)

#define ln_opck_param_array_int_ge(entry, expect)                       \
    do {                                                                \
        for (int i = 0; i < (entry)->array_len; i++)                    \
            ln_opck(LN_ERROR, (entry)->value_array_int[i] >= (expect),  \
                    "%s: `%s`'s `%s` param[%d] should be >= %d, but gets %d ", \
                    op_arg->optype, op_arg->name, (entry)->arg_name, i, \
                    (expect), (entry)->value_array_int[i]);             \
    } while (0)

#define ln_opck_param_array_int_lt(entry, expect)                       \
    do {                                                                \
        for (int i = 0; i < (entry)->array_len; i++)                    \
            ln_opck(LN_ERROR, (entry)->value_array_int[i] < (expect),   \
                    "%s: `%s`'s `%s` param[%d] should be < %d, but gets %d ", \
                    op_arg->optype, op_arg->name, (entry)->arg_name, i, \
                    (expect), (entry)->value_array_int[i]);             \
    } while (0)

#define ln_opck_param_array_int_le(entry, expect)                       \
    do {                                                                \
        for (int i = 0; i < (entry)->array_len; i++)                    \
            ln_opck(LN_ERROR, (entry)->value_array_int[i] <= (expect),  \
                    "%s: `%s`'s `%s` param[%d] should be <= %d, but gets %d ", \
                    op_arg->optype, op_arg->name, (entry)->arg_name, i, \
                    (expect), (entry)->value_array_int[i]);             \
    } while (0)

#define ln_opck_param_array_int_ne(entry, expect)                       \
    do {                                                                \
        for (int i = 0; i < (entry)->array_len; i++)                    \
            ln_opck(LN_ERROR, (entry)->value_array_int[i] != (expect),  \
                    "%s: `%s`'s `%s` param[%d] should be != %d, but gets %d ", \
                    op_arg->optype, op_arg->name, (entry)->arg_name, i, \
                    (expect), (entry)->value_array_int[i]);             \
    } while (0)

#define ln_opck_param_array_float_eq(entry, expect)                     \
    do {                                                                \
        for (int i = 0; i < (entry)->array_len; i++)                    \
            ln_opck(LN_ERROR, (entry)->value_array_float[i] == (expect), \
                    "%s: `%s`'s `%s` param[%d] should be == %f, but gets %f ", \
                    op_arg->optype, op_arg->name, (entry)->arg_name, i, \
                    (expect), (entry)->value_array_float[i]);           \
    } while (0)

#define ln_opck_param_array_float_gt(entry, expect)                     \
    do {                                                                \
        for (int i = 0; i < (entry)->array_len; i++)                    \
            ln_opck(LN_ERROR, (entry)->value_array_float[i] > (expect), \
                    "%s: `%s`'s `%s` param[%d] should be > %f, but gets %f ", \
                    op_arg->optype, op_arg->name, (entry)->arg_name, i, \
                    (expect), (entry)->value_array_float[i]);           \
    } while (0)

#define ln_opck_param_array_float_ge(entry, expect)                     \
    do {                                                                \
        for (int i = 0; i < (entry)->array_len; i++)                    \
            ln_opck(LN_ERROR, (entry)->value_array_float[i] >= (expect), \
                    "%s: `%s`'s `%s` param[%d] should be >= %f, but gets %f ", \
                    op_arg->optype, op_arg->name, (entry)->arg_name, i, \
                    (expect), (entry)->value_array_float[i]);           \
    } while (0)

#define ln_opck_param_array_float_lt(entry, expect)                     \
    do {                                                                \
        for (int i = 0; i < (entry)->array_len; i++)                    \
            ln_opck(LN_ERROR, (entry)->value_array_float[i] < (expect), \
                    "%s: `%s`'s `%s` param[%d] should be < %f, but gets %f ", \
                    op_arg->optype, op_arg->name, (entry)->arg_name, i, \
                    (expect), (entry)->value_array_float[i]);           \
    } while (0)

#define ln_opck_param_array_float_le(entry, expect)                     \
    do {                                                                \
        for (int i = 0; i < (entry)->array_len; i++)                    \
            ln_opck(LN_ERROR, (entry)->value_array_float[i] <= (expect), \
                    "%s: `%s`'s `%s` param[%d] should be <= %f, but gets %f ", \
                    op_arg->optype, op_arg->name, (entry)->arg_name, i, \
                    (expect), (entry)->value_array_float[i]);           \
    } while (0)

#define ln_opck_param_array_float_ne(entry, expect)                     \
    do {                                                                \
        for (int i = 0; i < (entry)->array_len; i++)                    \
            ln_opck(LN_ERROR, (entry)->value_array_float[i] != (expect), \
                    "%s: `%s`'s `%s` param[%d] should be != %f, but gets %f ", \
                    op_arg->optype, op_arg->name, (entry)->arg_name, i, \
                    (expect), (entry)->value_array_float[i]);           \
    } while (0)

#define ln_opck_param_array_double_eq(entry, expect)                    \
    do {                                                                \
        for (int i = 0; i < (entry)->array_len; i++)                    \
            ln_opck(LN_ERROR, (entry)->value_array_double[i] == (expect), \
                    "%s: `%s`'s `%s` param[%d] should be == %f, but gets %f ", \
                    op_arg->optype, op_arg->name, (entry)->arg_name, i, \
                    (expect), (entry)->value_array_double[i]);          \
    } while (0)

#define ln_opck_param_array_double_gt(entry, expect)                    \
    do {                                                                \
        for (int i = 0; i < (entry)->array_len; i++)                    \
            ln_opck(LN_ERROR, (entry)->value_array_double[i] > (expect), \
                    "%s: `%s`'s `%s` param[%d] should be > %f, but gets %f ", \
                    op_arg->optype, op_arg->name, (entry)->arg_name, i, \
                    (expect), (entry)->value_array_double[i]);          \
    } while (0)

#define ln_opck_param_array_double_ge(entry, expect)                    \
    do {                                                                \
        for (int i = 0; i < (entry)->array_len; i++)                    \
            ln_opck(LN_ERROR, (entry)->value_array_double[i] >= (expect), \
                    "%s: `%s`'s `%s` param[%d] should be >= %f, but gets %f ", \
                    op_arg->optype, op_arg->name, (entry)->arg_name, i, \
                    (expect), (entry)->value_array_double[i]);          \
    } while (0)

#define ln_opck_param_array_double_lt(entry, expect)                    \
    do {                                                                \
        for (int i = 0; i < (entry)->array_len; i++)                    \
            ln_opck(LN_ERROR, (entry)->value_array_double[i] < (expect), \
                    "%s: `%s`'s `%s` param[%d] should be < %f, but gets %f ", \
                    op_arg->optype, op_arg->name, (entry)->arg_name, i, \
                    (expect), (entry)->value_array_double[i]);          \
    } while (0)

#define ln_opck_param_array_double_le(entry, expect)                    \
    do {                                                                \
        for (int i = 0; i < (entry)->array_len; i++)                    \
            ln_opck(LN_ERROR, (entry)->value_array_double[i] <= (expect), \
                    "%s: `%s`'s `%s` param[%d] should be <= %f, but gets %f ", \
                    op_arg->optype, op_arg->name, (entry)->arg_name, i, \
                    (expect), (entry)->value_array_double[i]);          \
    } while (0)

#define ln_opck_param_array_double_ne(entry, expect)                    \
    do {                                                                \
        for (int i = 0; i < (entry)->array_len; i++)                    \
            ln_opck(LN_ERROR, (entry)->value_array_double[i] != (expect), \
                    "%s: `%s`'s `%s` param[%d] should be != %f, but gets %f ", \
                    op_arg->optype, op_arg->name, (entry)->arg_name, i, \
                    (expect), (entry)->value_array_double[i]);          \
    } while (0)

/* list_len should be returned by ln_param_list_length(op_arg->params) */
#define ln_opck_params_len_eq(list_len, expect_len)                     \
    ln_opck(LN_ERROR, (list_len) == (expect_len),                       \
            "%s: `%s` needs %d params, but gets %d params",             \
            op_arg->optype, op_arg->name, (expect_len), (list_len))

/* list_len should be returned by ln_param_list_length(op_arg->params) */
#define ln_opck_params_len_gt(list_len, expect_len)                     \
    ln_opck(LN_ERROR, (list_len) > (expect_len),                        \
            "%s: `%s` needs > %d params, but gets %d params",           \
            op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_params_len_ge(list_len, expect_len)                     \
    ln_opck(LN_ERROR, (list_len) >= (expect_len),                       \
            "%s: `%s` needs >= %d params, but gets %d params",          \
            op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_params_len_lt(list_len, expect_len)                     \
    ln_opck(LN_ERROR, (list_len) < (expect_len),                        \
            "%s: `%s` needs < %d params, but gets %d params",           \
            op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_params_len_le(list_len, expect_len)                     \
    ln_opck(LN_ERROR, (list_len) <= (expect_len),                       \
            "%s: `%s` needs <= %d params, but gets %d params",          \
            op_arg->optype, op_arg->name, (expect_len), (list_len))

/* tle should be returned by
   ln_tensor_list_find_by_arg_name(op_arg->tensors_in, arg_name) */
#define ln_opck_tensor_in_exist(tle, arg_name)          \
    ln_opck(LN_ERROR, (tle),                            \
            "%s: `%s` needs a `%s` input tensor",       \
            op_arg->optype, op_arg->name, (arg_name))

/* list_len should be returned by ln_tensor_list_length(op_arg->tensors_in) */
#define ln_opck_tensors_in_len_eq(list_len, expect_len)                 \
    ln_opck(LN_ERROR, (list_len) == (expect_len),                       \
            "%s: `%s` needs %d input tensors, but gets %d input tensors", \
            op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_tensors_in_len_gt(list_len, expect_len)                 \
    ln_opck(LN_ERROR, (list_len) > (expect_len),                        \
            "%s: `%s` needs > %d input tensors, but gets %d input tensors", \
            op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_tensors_in_len_ge(list_len, expect_len)                 \
    ln_opck(LN_ERROR, (list_len) >= (expect_len),                       \
            "%s: `%s` needs >= %d input tensors, but gets %d input tensors", \
            op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_tensors_in_len_lt(list_len, expect_len)                 \
    ln_opck(LN_ERROR, (list_len) < (expect_len),                        \
            "%s: `%s` needs < %d input tensors, but gets %d input tensors", \
            op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_tensors_in_len_le(list_len, expect_len)                 \
    ln_opck(LN_ERROR, (list_len) <= (expect_len),                       \
            "%s: `%s` needs <= %d input tensors, but gets %d input tensors", \
            op_arg->optype, op_arg->name, (expect_len), (list_len))

/* tle should be returned by
   ln_tensor_list_find_by_arg_name(op_arg->tensors_out, arg_name) */
#define ln_opck_tensor_out_exist(tle, arg_name)         \
    ln_opck(LN_ERROR, (tle),                            \
            "%s: `%s` needs a `%s` output tensor",      \
            op_arg->optype, op_arg->name, (arg_name))

/* list_len should be returned by ln_tensor_list_length(op_arg->tensors_out) */
#define ln_opck_tensors_out_len_eq(list_len, expect_len)                \
    ln_opck(LN_ERROR, (list_len) == (expect_len),                       \
            "%s: `%s` needs %d output tensors, but gets %d output tensors", \
            op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_tensors_out_len_gt(list_len, expect_len)                \
    ln_opck(LN_ERROR, (list_len) > (expect_len),                        \
            "%s: `%s` needs > %d output tensors, but gets %d output tensors", \
            op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_tensors_out_len_ge(list_len, expect_len)                \
    ln_opck(LN_ERROR, (list_len) >= (expect_len),                       \
            "%s: `%s` needs >= %d output tensors, but gets %d output tensors", \
            op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_tensors_out_len_lt(list_len, expect_len)                \
    ln_opck(LN_ERROR, (list_len) < (expect_len),                        \
            "%s: `%s` needs < %d output tensors, but gets %d output tensors", \
            op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_tensors_out_len_le(list_len, expect_len)                \
    ln_opck(LN_ERROR, (list_len) <= (expect_len),                       \
            "%s: `%s` needs <= %d output tensors, but gets %d output tensors", \
            op_arg->optype, op_arg->name, (expect_len), (list_len))

/* entry should be returned by ln_tensor_table_find(op_arg->tensor_table, name) */
#define ln_opck_tensor_not_defined(entry, entry_name)                   \
    ln_opck(LN_ERROR, !(entry),                                         \
            "%s: `%s`'s tensor `%s` shouldn't have been defined before", \
            op_arg->optype, op_arg->name, (entry_name))

#define ln_opck_tensor_defined(entry, entry_name)                       \
    ln_opck(LN_ERROR, (entry),                                          \
            "%s: `%s`'s tensor `%s` should have been defined before",   \
            op_arg->optype, op_arg->name, (entry_name))

/* entry should have been checked with ln_opck_tensor_defined */
#define ln_opck_tensor_ndim(entry, expect_ndim)                         \
    ln_opck(LN_ERROR, (entry)->tensor->ndim == (expect_ndim),           \
            "%s: `%s`'s tensor `%s` should be a %d-dimensional tensor", \
            op_arg->optype, op_arg->name, (entry)->name, expect_ndim)

/* entry should have been checked with ln_opck_tensor_defined */
#define ln_opck_tensor_len(entry, expect_len)                           \
    ln_opck(LN_ERROR, (entry)->tensor->len == (expect_len),             \
            "%s: `%s`'s tensor `%s` should have %d elements",           \
            op_arg->optype, op_arg->name, (entry)->name, expect_len)

/* entry1 and entry2 should have been checked with ln_opck_tensor_defined */
#define ln_opck_tensor_issameshape(entry1, entry2)                      \
    ln_opck(LN_ERROR, tl_tensor_issameshape((entry1)->tensor, (entry2)->tensor), \
            "%s: `%s`'s tensor `%s` and tensor `%s` should have the same shape", \
            op_arg->optype, op_arg->name, (entry1)->name, (entry2)->name)

#define ln_opck_tensor_issametype(entry1, entry2)                       \
    ln_opck(LN_ERROR, (entry1)->tensor->dtype == (entry2)->tensor->dtype, \
            "%s: `%s`'s tensor `%s` and tensor `%s` should have the same data type", \
            op_arg->optype, op_arg->name, (entry1)->name, (entry2)->name)

#define ln_opck_tensor_isstatic(entry)                          \
    ln_opck(LN_ERROR, (entry)->isstatic,                        \
            "%s: `%s`'s tensor `%s` should be static",          \
            op_arg->optype, op_arg->name, (entry)->name)

#define ln_opck_tensor_isnotstatic(entry)                       \
    ln_opck(LN_ERROR, !(entry)->isstatic,                       \
            "%s: `%s`'s tensor `%s` should not be static",	\
            op_arg->optype, op_arg->name, (entry)->name)

#define ln_opck_tensor_mtype_eq(entry, mem_type)                        \
    ln_opck(LN_ERROR, (entry)->mtype == (mem_type),                     \
            "%s: `%s`'s tensor `%s`'s mtype should be %s, but gets %s", \
            op_arg->optype, op_arg->name, (entry)->name, ln_mem_type_name(mem_type), \
            ln_mem_type_name((entry)->mtype))

#define ln_opck_tensor_dtype_eq(entry, data_type)                       \
    ln_opck(LN_ERROR, (entry)->tensor->dtype == (data_type),            \
            "%s: `%s`'s tensor `%s`'s dtype should be %s, but gets %s", \
            op_arg->optype, op_arg->name, (entry)->name, tl_dtype_name(data_type), \
            tl_dtype_name((entry)->tensor->dtype))

#endif  /* _LN_OP_H_ */

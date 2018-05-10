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

#include "ln_tensor.h"
#include "ln_param.h"
#include "ln_error.h"

typedef struct ln_op_arg ln_op_arg;
struct ln_op_arg {
     char            *name;
     char            *optype;
     ln_tensor_table *tensors;
     ln_param_table  *params;
};

typedef void (*ln_op_func) (ln_op_arg *op_arg, ln_error **error);

typedef struct ln_op ln_op;
struct ln_op {
     ln_op_arg   *op_arg;
     ln_op_func   pre_run;
     ln_op_func   run;
     ln_op_func   post_run;
};

#ifdef __cplusplus
extern "C" {
#endif

     ln_op *ln_op_create(const char *name, const char *optype,
                         ln_tensor_table *tensors, ln_param_table *params,
                         ln_op_func pre_run, ln_op_func run, ln_op_func post_run);
     void ln_op_free(ln_op *op);
     void ln_op_list_free_tables_too(ln_list *ops);
     tl_tensor *ln_op_list_find_tensor_by_name(ln_list *ops, char *name);
     ln_op *ln_op_list_find_by_optype(ln_list *ops, char *optype);
     void ln_op_list_do_run(ln_list *ops, ln_error **error);
     void ln_op_list_do_post_run(ln_list *ops, ln_error **error);

#ifdef __cplusplus
}
#endif

/*
 * Convenient macros for checking parameters in a ln_op_func,
 * usually in a ln_op->pre_run function.
 * *level* is an enum defined in ln_error.h
 * NOTE: If there is more error handling work, please write the code yourself
 * instead of using those macros.
 * NOTE: Normally we shouldn't use those kind of error handling routines in
 * ln_op->run and ln_op->post_run functions, where errors should be considered
 * as bugs.
 */
#define ln_op_check(level, condition, msg_fmt, varg...)			\
     do {								\
	  if (!(condition)) {						\
	       *error = ln_error_create((level), (msg_fmt), ##varg);	\
	       return;							\
	  }								\
     } while(0)

#define ln_op_check_param_satisfy_msg(level, condition, msg)    \
     ln_op_check(level, condition,                              \
                 "%s: \"%s\"'s params should satisfy \"%s\"",   \
                 op_arg->optype, op_arg->name, (msg))

/* condition is appended as the message */
#define ln_op_check_param_satisfy(level, condition)                     \
     ln_op_check_param_satisfy_msg(level, condition, #condition)

/* entry should be returned by
   ln_param_table_find_by_arg_name(op_arg->params, arg_name) */
#define ln_op_check_param_exist(level, entry, arg_name)         \
     ln_op_check(level, entry,                                  \
                 "%s: \"%s\" needs a \"%s\" param",		\
                 op_arg->optype, op_arg->name, (arg_name))

/*
 * entry should have been checked with ln_op_check_param_exist
 * type is an enum defined in ln_param.h
 */
#define ln_op_check_param_type(level, entry, param_type)                \
     ln_op_check(level, entry->type == param_type,                      \
                 "%s: \"%s\"'s \"%s\" param's value should be of type %s, but got a %s", \
                 op_arg->optype, op_arg->name, entry->arg_name,         \
                 ln_param_type_name(param_type), ln_param_type_name(entry->type))

/* table_length should be returned by ln_param_table_length(op_arg->params) */
#define ln_op_check_param_num_eq(level, table_length, num)              \
     ln_op_check(level, table_length == num,                            \
                 "%s: \"%s\" needs %d params, but got %d params",       \
                 op_arg->optype, op_arg->name, (num), (table_length))

/* table_length should be returned by ln_param_table_length(op_arg->params) */
#define ln_op_check_param_num_gt(level, table_length, num)              \
     ln_op_check(level, table_length > num,                             \
                 "%s: \"%s\" needs > %d params, but got %d params",     \
                 op_arg->optype, op_arg->name, (num), (table_length))

/* table_length should be returned by ln_param_table_length(op_arg->params) */
#define ln_op_check_param_num_ge(level, table_length, num)              \
     ln_op_check(level, table_length >= num,                            \
                 "%s: \"%s\" needs >= %d params, but got %d params",    \
                 op_arg->optype, op_arg->name, (num), (table_length))

/* table_length should be returned by ln_param_table_length(op_arg->params) */
#define ln_op_check_param_num_lt(level, table_length, num)              \
     ln_op_check(level, table_length < num,                             \
                 "%s: \"%s\" needs < %d params, but got %d params",     \
                 op_arg->optype, op_arg->name, (num), (table_length))

/* table_length should be returned by ln_param_table_length(op_arg->params) */
#define ln_op_check_param_num_le(level, table_length, num)              \
     ln_op_check(level, table_length <= num,                            \
                 "%s: \"%s\" needs <= %d params, but got %d params",    \
                 op_arg->optype, op_arg->name, (num), (table_length))

#define ln_op_check_tensor_satisfy_msg(level, condition, msg)   \
     ln_op_check(level, condition,                              \
                 "%s: \"%s\"'s tensors should satisfy \"%s\"",  \
                 op_arg->optype, op_arg->name, (msg))

/* condition is appended as the message */
#define ln_op_check_tensor_satisfy(level, condition)                    \
     ln_op_check_tensor_satisfy_msg(level, condition, #condition)

/* entry should be returned by
   ln_tensor_table_find_by_arg_name(op_arg->tensors, arg_name) */
#define ln_op_check_tensor_exist(level, entry, arg_name)	\
     ln_op_check(level, entry,                                  \
                 "%s: \"%s\" needs a \"%s\" tensor",		\
                 op_arg->optype, op_arg->name, (arg_name))

/* table_length should be returned by ln_tensor_table_length(op_arg->tensors) */
#define ln_op_check_tensor_num_eq(level, table_length, num)             \
     ln_op_check(level, table_length == num,                            \
                 "%s: \"%s\" needs %d tensors, but got %d tensors",     \
		 op_arg->optype, op_arg->name, (num), (table_length))

/* table_length should be returned by ln_tensor_table_length(op_arg->tensors) */
#define ln_op_check_tensor_num_gt(level, table_length, num)             \
     ln_op_check(level, table_length > num,                             \
                 "%s: \"%s\" needs > %d tensors, but got %d tensors",   \
		 op_arg->optype, op_arg->name, (num), (table_length))

/* table_length should be returned by ln_tensor_table_length(op_arg->tensors) */
#define ln_op_check_tensor_num_ge(level, table_length, num)             \
     ln_op_check(level, table_length >= num,                            \
                 "%s: \"%s\" needs >= %d tensors, but got %d tensors",  \
		 op_arg->optype, op_arg->name, (num), (table_length))

/* table_length should be returned by ln_tensor_table_length(op_arg->tensors) */
#define ln_op_check_tensor_num_lt(level, table_length, num)             \
     ln_op_check(level, table_length < num,                             \
                 "%s: \"%s\" needs < %d tensors, but got %d tensors",   \
		 op_arg->optype, op_arg->name, (num), (table_length))

/* table_length should be returned by ln_tensor_table_length(op_arg->tensors) */
#define ln_op_check_tensor_num_le(level, table_length, num)             \
     ln_op_check(level, table_length <= num,                            \
                 "%s: \"%s\" needs <= %d tensors, but got %d tensors",  \
		 op_arg->optype, op_arg->name, (num), (table_length))

/* length should have been checked with ln_op_check_tensor_exist */
#define ln_op_check_tensor_not_defined(level, entry)                    \
     ln_op_check(level, !entry->tensor,                                 \
		 "%s: \"%s\"'s \"%s\" tensor \"%s\" has been defined before", \
		 op_arg->optype, op_arg->name, entry->arg_name, entry->name)

/* entry should have been checked with ln_op_check_tensor_exist */
#define ln_op_check_tensor_defined(level, entry)                        \
     ln_op_check(level, entry->tensor,					\
		 "%s: \"%s\"'s \"%s\" tensor \"%s\" has not been defined before", \
		 op_arg->optype, op_arg->name, entry->arg_name, entry->name)

#endif  /* _LN_OP_H_ */

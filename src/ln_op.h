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

#include "ln_mem.h"
#include "ln_tensor.h"
#include "ln_param.h"
#include "ln_error.h"

struct ln_op_arg {
     char            *name;
     char            *optype;
     ln_list         *tensors_in;
     ln_list         *tensors_out;
     ln_list         *params;
     void            *priv;     /* for other private data storage */
     ln_hash         *tensor_table;
     int              fixed;    /* won't be replaced if set to 1 */
};
typedef struct ln_op_arg ln_op_arg;

typedef void (*ln_op_func) (ln_op_arg *op_arg, ln_error **error);

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
void ln_op_free_lists_too(ln_op *op);
ln_list *ln_op_list_create_from_array(ln_op **op_array);
void ln_op_list_free(ln_list *op_list);
void ln_op_list_free_lists_too(ln_list *ops);
ln_op *ln_op_list_find_by_optype(ln_list *ops, char *optype);
ln_op *ln_op_array_find_by_optype(ln_op *ops[], char *optype);
ln_op *ln_op_list_find_by_name(ln_list *ops, char *name);
void ln_op_list_do_pre_run(ln_list *ops, ln_error **error);
void ln_op_list_do_static_run(ln_list *ops, ln_error **error);
void ln_op_list_do_run(ln_list *ops, ln_error **error);
void ln_op_list_do_post_run(ln_list *ops, ln_error **error);

#ifdef __cplusplus
LN_CPPEND
#endif

/*
 * Convenient macros for operator parameter checking in a ln_op_func,
 * usually in a ln_op->pre_run function.
 * *level* is an enum defined in ln_error.h
 * NOTE: If there is more error handling work, please write the code yourself
 * instead of using those macros.
 * NOTE: Normally we shouldn't use those kind of error handling routines in
 * ln_op->run and ln_op->post_run functions, where errors should be considered
 * as bugs.
 */
#define ln_opck(level, condition, msg_fmt, varg...)                     \
     do {                                                               \
          if (!(condition)) {                                           \
               *error = ln_error_create((level), (msg_fmt), ##varg);	\
               if ((level) == LN_WARNING || (level) == LN_WARNING_SYS || \
                   (level) == LN_INFO) {                                \
                    ln_error_handle(error);                             \
                    break;                                              \
               }                                                        \
               return;                                                  \
          }                                                             \
     } while (0)

#define ln_opck_param_satisfy_msg(condition, msg)       \
     ln_opck(LN_ERROR, (condition),                     \
             "%s: \"%s\"'s params should satisfy: %s",  \
             op_arg->optype, op_arg->name, (msg))

/* condition is appended as the message */
#define ln_opck_param_satisfy(condition)                \
     ln_opck_param_satisfy_msg((condition), #condition)

#define ln_opck_param_error(condition, msg)             \
     ln_opck(LN_ERROR, (condition),                     \
             "%s: \"%s\"'s param error: %s",            \
             op_arg->optype, op_arg->name, (msg))

#define ln_opck_param_warning(condition, msg)           \
     ln_opck(LN_WARNING, (condition),                   \
             "%s: \"%s\"'s param warning: %s",          \
             op_arg->optype, op_arg->name, (msg))

/* entry should be returned by
   ln_param_list_find(op_arg->params, arg_name) */
#define ln_opck_param_exist(entry, arg_name)            \
     ln_opck(LN_ERROR, (entry),                         \
             "%s: \"%s\" needs a \"%s\" param",		\
             op_arg->optype, op_arg->name, (arg_name))

/*
 * entry should have been checked with ln_opck_param_exist
 * type is an enum defined in ln_param.h
 */
#define ln_opck_param_type(entry, param_type)                           \
     ln_opck(LN_ERROR, (entry)->type == (param_type),                   \
             "%s: \"%s\"'s \"%s\" param's value should be of type %s, but gets a %s", \
             op_arg->optype, op_arg->name, (entry)->arg_name,           \
             ln_param_type_name((param_type)), ln_param_type_name((entry)->type))

/* entry should have been checked with ln_opck_param_exist */
#define ln_opck_param_array_len_eq(entry, expect_len)                   \
     ln_opck(LN_ERROR, (entry)->array_len == (expect_len),              \
             "%s: \"%s\"'s \"%s\" param needs %d elements, but gets %d elements", \
             op_arg->optype, op_arg->name, (entry)->arg_name,           \
             (expect_len), (entry)->array_len)

#define ln_opck_param_array_len_gt(entry, expect_len)                   \
     ln_opck(LN_ERROR, (entry)->array_len > (expect_len),               \
             "%s: \"%s\"'s \"%s\" param needs > %d elements, but gets %d elements", \
             op_arg->optype, op_arg->name, (entry)->arg_name,           \
             (expect_len), (entry)->array_len)

#define ln_opck_param_array_len_ge(entry, expect_len)                   \
     ln_opck(LN_ERROR, (entry)->array_len >= (expect_len),              \
             "%s: \"%s\"'s \"%s\" param needs >= %d elements, but gets %d elements", \
             op_arg->optype, op_arg->name, (entry)->arg_name,           \
             (expect_len), (entry)->array_len)

#define ln_opck_param_array_len_lt(entry, expect_len)                   \
     ln_opck(LN_ERROR, (entry)->array_len < (expect_len),               \
             "%s: \"%s\"'s \"%s\" param needs < %d elements, but gets %d elements", \
             op_arg->optype, op_arg->name, (entry)->arg_name,           \
             (expect_len), (entry)->array_len)

#define ln_opck_param_array_len_le(entry, expect_len)                   \
     ln_opck(LN_ERROR, (entry)->array_len <= (expect_len),              \
             "%s: \"%s\"'s \"%s\" param needs <= %d elements, but gets %d elements", \
             op_arg->optype, op_arg->name, (entry)->arg_name,           \
             (expect_len), (entry)->array_len)

/* list_len should be returned by ln_param_list_length(op_arg->params) */
#define ln_opck_param_len_eq(list_len, expect_len)                      \
     ln_opck(LN_ERROR, (list_len) == (expect_len),                      \
             "%s: \"%s\" needs %d params, but gets %d params",          \
             op_arg->optype, op_arg->name, (expect_len), (list_len))

/* list_len should be returned by ln_param_list_length(op_arg->params) */
#define ln_opck_param_len_gt(list_len, expect_len)                      \
     ln_opck(LN_ERROR, (list_len) > (expect_len),                       \
             "%s: \"%s\" needs > %d params, but gets %d params",        \
             op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_param_len_ge(list_len, expect_len)                      \
     ln_opck(LN_ERROR, (list_len) >= (expect_len),                      \
             "%s: \"%s\" needs >= %d params, but gets %d params",       \
             op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_param_len_lt(list_len, expect_len)                      \
     ln_opck(LN_ERROR, (list_len) < (expect_len),                       \
             "%s: \"%s\" needs < %d params, but gets %d params",        \
             op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_param_len_le(list_len, expect_len)                      \
     ln_opck(LN_ERROR, (list_len) <= (expect_len),                      \
             "%s: \"%s\" needs <= %d params, but gets %d params",       \
             op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_tensor_satisfy_msg(condition, msg)      \
     ln_opck(LN_ERROR, (condition),                     \
             "%s: \"%s\"'s tensors should satisfy: %s", \
             op_arg->optype, op_arg->name, (msg))

/* condition is appended as the message */
#define ln_opck_tensor_satisfy(condition)                       \
     ln_opck_tensor_satisfy_msg((condition), #condition)

/* name should be returned by
   ln_tensor_list_find_name(op_arg->tensors_in, arg_name) */
#define ln_opck_tensor_in_exist(tensor_name, arg_name)	\
     ln_opck(LN_ERROR, (tensor_name),                   \
             "%s: \"%s\" needs a \"%s\" input tensor",  \
             op_arg->optype, op_arg->name, (arg_name))

/* list_len should be returned by ln_tensor_list_length(op_arg->tensors_in) */
#define ln_opck_tensor_in_len_eq(list_len, expect_len)                  \
     ln_opck(LN_ERROR, (list_len) == (expect_len),                      \
             "%s: \"%s\" needs %d input tensors, but gets %d input tensors", \
             op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_tensor_in_len_gt(list_len, expect_len)                  \
     ln_opck(LN_ERROR, (list_len) > (expect_len),                       \
             "%s: \"%s\" needs > %d input tensors, but gets %d input tensors", \
             op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_tensor_in_len_ge(list_len, expect_len)                  \
     ln_opck(LN_ERROR, (list_len) >= (expect_len),                      \
             "%s: \"%s\" needs >= %d input tensors, but gets %d input tensors", \
             op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_tensor_in_len_lt(list_len, expect_len)                  \
     ln_opck(LN_ERROR, (list_len) < (expect_len),                       \
             "%s: \"%s\" needs < %d input tensors, but gets %d input tensors", \
             op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_tensor_in_len_le(list_len, expect_len)                  \
     ln_opck(LN_ERROR, (list_len) <= (expect_len),                      \
             "%s: \"%s\" needs <= %d input tensors, but gets %d input tensors", \
             op_arg->optype, op_arg->name, (expect_len), (list_len))

/* name should be returned by
   ln_tensor_list_find_name(op_arg->tensors_out, arg_name) */
#define ln_opck_tensor_out_exist(tensor_name, arg_name)	\
     ln_opck(LN_ERROR, (tensor_name),                   \
             "%s: \"%s\" needs a \"%s\" output tensor", \
             op_arg->optype, op_arg->name, (arg_name))

/* list_len should be returned by ln_tensor_list_length(op_arg->tensors_out) */
#define ln_opck_tensor_out_len_eq(list_len, expect_len)                 \
     ln_opck(LN_ERROR, (list_len) == (expect_len),                      \
             "%s: \"%s\" needs %d output tensors, but gets %d output tensors", \
             op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_tensor_out_len_gt(list_len, expect_len)                 \
     ln_opck(LN_ERROR, (list_len) > (expect_len),                       \
             "%s: \"%s\" needs > %d output tensors, but gets %d output tensors", \
             op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_tensor_out_len_ge(list_len, expect_len)                 \
     ln_opck(LN_ERROR, (list_len) >= (expect_len),                      \
             "%s: \"%s\" needs >= %d output tensors, but gets %d output tensors", \
             op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_tensor_out_len_lt(list_len, expect_len)                 \
     ln_opck(LN_ERROR, (list_len) < (expect_len),                       \
             "%s: \"%s\" needs < %d output tensors, but gets %d output tensors", \
             op_arg->optype, op_arg->name, (expect_len), (list_len))

#define ln_opck_tensor_out_len_le(list_len, expect_len)                 \
     ln_opck(LN_ERROR, (list_len) <= (expect_len),                      \
             "%s: \"%s\" needs <= %d output tensors, but gets %d output tensors", \
             op_arg->optype, op_arg->name, (expect_len), (list_len))

/* entry should be returned by ln_tensor_table_find(op_arg->tensor_table, name) */
#define ln_opck_tensor_not_defined(entry, entry_name)                   \
     ln_opck(LN_ERROR, !(entry),                                        \
             "%s: \"%s\"'s tensor \"%s\" shouldn't have been defined before", \
             op_arg->optype, op_arg->name, (entry_name))

#define ln_opck_tensor_defined(entry, entry_name)                       \
     ln_opck(LN_ERROR, (entry),                                         \
             "%s: \"%s\"'s tensor \"%s\" should have been defined before", \
             op_arg->optype, op_arg->name, (entry_name))

/* entry1 and entry2 should have been checked with ln_opck_tensor_defined */
#define ln_opck_tensor_issameshape(entry1, entry2)                      \
     ln_opck(LN_ERROR, tl_tensor_issameshape((entry1)->tensor, (entry2)->tensor), \
             "%s: \"%s\"'s tensor \"%s\" and tensor \"%s\" should have the same shape", \
             op_arg->optype, op_arg->name, (entry1)->name, (entry2)->name)

#define ln_opck_tensor_issametype(entry1, entry2)                       \
     ln_opck(LN_ERROR, (entry1)->tensor->dtype == (entry2)->tensor->dtype, \
             "%s: \"%s\"'s tensor \"%s\" and tensor \"%s\" should be the same data type", \
             op_arg->optype, op_arg->name, (entry1)->name, (entry2)->name)

#define ln_opck_tensor_isstatic(entry)                          \
     ln_opck(LN_ERROR, (entry)->isstatic,                       \
             "%s: \"%s\"'s tensor \"%s\" should be static",     \
             op_arg->optype, op_arg->name, (entry)->name)

#define ln_opck_tensor_isnotstatic(entry)				\
	ln_opck(LN_ERROR, !(entry)->isstatic,				\
		"%s: \"%s\"'s tensor \"%s\" should not be static",	\
		op_arg->optype, op_arg->name, (entry)->name)

#define ln_opck_tensor_mtype_eq(entry, mem_type)                        \
     ln_opck(LN_ERROR, (entry)->mtype == (mem_type),                    \
             "%s: \"%s\"'s tensor \"%s\"'s mtype should be %s, but gets %s", \
             op_arg->optype, op_arg->name, (entry)->name, ln_mem_type_name(mem_type), \
             ln_mem_type_name((entry)->mtype))

#endif  /* _LN_OP_H_ */

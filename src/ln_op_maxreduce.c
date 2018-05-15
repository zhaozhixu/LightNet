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
#include "ln_op.h"

/*
 * This function should do the parameter checking and tensor memory allocation.
 */
static void maxreduce_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *src_entry, *dst_entry, *arg_entry;
     ln_param_entry *axis_entry;
     int tensors_n, params_n;
     int axis;

     /* check tensors and parameters */
     tensors_n = ln_tensor_table_length(op_arg->tensors);
     ln_op_check_tensor_len_ge(LN_ERROR, tensors_n, 2);
     ln_op_check_tensor_len_le(LN_ERROR, tensors_n, 3);

     src_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "src");
     ln_op_check_tensor_exist(LN_ERROR, src_entry, "src");
     ln_op_check_tensor_defined(LN_ERROR, src_entry);

     dst_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "dst");
     ln_op_check_tensor_exist(LN_ERROR, dst_entry, "dst");
     ln_op_check_tensor_not_defined(LN_ERROR, dst_entry);

     /* "arg" is an optional parameter */
     arg_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "arg");
     if (arg_entry)
          ln_op_check_tensor_not_defined(LN_ERROR, arg_entry);

     params_n = ln_param_table_length(op_arg->params);
     ln_op_check_param_len_eq(LN_ERROR, params_n, 1);

     axis_entry = ln_param_table_find_by_arg_name(op_arg->params, "axis");
     ln_op_check_param_type(LN_ERROR, axis_entry, LN_PARAM_NUMBER);

     axis = axis_entry->value_int;
     ln_op_check_param_satisfy(LN_ERROR,
                               axis >= 0 && axis < src_entry->tensor->ndim);

     /* allocate tensor memory in need */
     dst_entry->tensor = tl_tensor_create_slice(src_entry->tensor, axis, 1,
                                                src_entry->tensor->dtype);
     if (arg_entry)
          arg_entry->tensor = tl_tensor_create_slice(src_entry->tensor, axis, 1,
                                                     src_entry->tensor->dtype);
}

/*
 * Normally we should only do the calculations here. Operations with memory
 * and such should go in pre_run().
 */
static void maxreduce_run(ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *src_entry, *dst_entry, *arg_entry;
     ln_param_entry *axis_entry;

     /* Get tensors and parameters, which should have been checked in pre_run().
        Further errors should be considered as bugs, so we use asserts to catch
        return value. */
     src_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "src");
     assert(src_entry);
     dst_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "dst");
     assert(dst_entry);
     arg_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "arg");
     axis_entry = ln_param_table_find_by_arg_name(op_arg->params, "axis");
     assert(axis_entry);

     /* do the real work */
     if (arg_entry)
          tl_tensor_maxreduce(src_entry->tensor, dst_entry->tensor,
                              arg_entry->tensor, axis_entry->value_int);
     else
          tl_tensor_maxreduce(src_entry->tensor, dst_entry->tensor,
                              NULL, axis_entry->value_int);
}

/*
 * This function should free all tensor memory pre_run() and run() allocated.
 */
static void maxreduce_post_run(ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *dst_entry, *arg_entry;

     dst_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "dst");
     assert(dst_entry);
     arg_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "arg");

     /* free the tensor memory allocated in pre_run() and run() */
     tl_tensor_free_data_too(dst_entry->tensor);
     if (arg_entry)
          tl_tensor_free_data_too(arg_entry->tensor);
}

static ln_op_arg op_arg_maxreduce = {
     .name = NULL,
     .optype = "maxreduce",
     .tensors = NULL,
     .params = NULL,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_op_maxreduce = {
     .op_arg = &op_arg_maxreduce,
     .pre_run = maxreduce_pre_run,
     .run = maxreduce_run,
     .post_run = maxreduce_post_run
};

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

static const int elew_op_size = 7;
static const char *elew_op_types[] = {
     "TL_MUL",
     "TL_DIV",
     "TL_SUM",
     "TL_SUB",
     "TL_MAX",
     "TL_MIN",
     "TL_POW"
};

static int is_str_in_array(char *str, char **array, int size)
{
     int i;

     for (i = 0; i < size; i++) {
          if (!strcmp(str, array[i]))
               return 1;
     }
     return 0;
}

/*
 * This function should do the parameter checking and tensor memory allocation.
 */
static void elew_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *src1_entry, *src2_entry, *dst_entry;
     ln_param_entry *elew_op_entry;
     int tensors_n, params_n;
     char *elew_op;

     /* check tensors and parameters */
     tensors_n = ln_tensor_table_length(op_arg->tensors);
     ln_op_check_tensor_num_eq(LN_ERROR, tensors_n, 3);

     src1_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "src1");
     ln_op_check_tensor_exist(LN_ERROR, src1_entry, "src1");
     ln_op_check_tensor_defined(LN_ERROR, src1_entry);

     src2_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "src2");
     ln_op_check_tensor_exist(LN_ERROR, src2_entry, "src2");
     ln_op_check_tensor_defined(LN_ERROR, src2_entry);

     dst_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "dst");
     ln_op_check_tensor_exist(LN_ERROR, dst_entry, "dst");
     ln_op_check_tensor_not_defined(LN_ERROR, dst_entry);

     params_n = ln_param_table_length(op_arg->params);
     ln_op_check_param_num_eq(LN_ERROR, params_n, 1);

     elew_op_entry = ln_param_table_find_by_arg_name(op_arg->params, "elew_op");
     ln_op_check_param_type(LN_ERROR, elew_op_entry, LN_PARAM_STRING);

     elew_op = elew_op_entry->value_string;
     ln_op_check_param_satisfy_msg(LN_ERROR,
                                   is_str_in_array(elew_op, elew_op_types, elew_op_size),
                                   "\"elew_op\" param \"\"");
     /* allocate tensor memory in need */
     /* ...... */
}

/*
 * Normally we should only do the calculations here. Operations with memory
 * and such should go in pre_run().
 */
static void elew_run(ln_op_arg *op_arg, ln_error **error)
{

     /* Get tensors and parameters, which should have been checked in pre_run().
        Further errors should be considered as bugs, so we use asserts to catch
        return value. */
     /* ...... */

     /* do the real work */
     /* ...... */
}

/*
 * This function should free all tensor memory pre_run() and run() allocated.
 */
static void elew_post_run(ln_op_arg *op_arg, ln_error **error)
{

     /* free the tensor memory allocated in pre_run() and run() */
     /* ..... */
}

static ln_op_arg op_arg_elew = {
     .name = NULL,
     .optype = "elew",
     .tensors = NULL,
     .params = NULL,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_op_elew = {
     .op_arg = &op_arg_elew,
     .pre_run = elew_pre_run,
     .run = elew_run,
     .post_run = elew_post_run
};

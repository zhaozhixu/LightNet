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

static int k2v(char *str)
{
     if (!strcmp(str, "TL_FLOAT"))
          return TL_FLOAT;
     if (!strcmp(str, "TL_INT32"))
          return TL_INT32;
     if (!strcmp(str, "TL_INT16"))
          return TL_INT16;
     if (!strcmp(str, "TL_INT8"))
          return TL_INT8;
     if (!strcmp(str, "TL_UINT32"))
          return TL_UINT32;
     if (!strcmp(str, "TL_UINT16"))
          return TL_UINT16;
     if (!strcmp(str, "TL_UINT8"))
          return TL_UINT8;
     if (!strcmp(str, "TL_BOOL"))
          return TL_BOOL;
     return -1;
}

/*
 * This function should do the parameter checking and tensor memory allocation.
 */
static void zeros_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *dst_entry;
     ln_param_entry *dtype_entry, *dims_entry;
     int tensors_n, params_n;
     int dtype, i;

     /* check tensors and parameters */
     tensors_n = ln_tensor_table_length(op_arg->tensors_in);
     ln_op_check_tensor_in_len_eq(LN_ERROR, tensors_n, 0);

     tensors_n = ln_tensor_table_length(op_arg->tensors_out);
     ln_op_check_tensor_out_len_eq(LN_ERROR, tensors_n, 1);

     dst_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors_out, "dst");
     ln_op_check_tensor_out_exist(LN_ERROR, dst_entry, "dst");
     ln_op_check_tensor_not_defined(LN_ERROR, dst_entry);

     params_n = ln_param_table_length(op_arg->params);
     ln_op_check_param_len_eq(LN_ERROR, params_n, 2);

     dtype_entry = ln_param_table_find_by_arg_name(op_arg->params, "dtype");
     ln_op_check_param_exist(LN_ERROR, dtype_entry, "dtype");
     ln_op_check_param_type(LN_ERROR, dtype_entry, LN_PARAM_STRING);

     dtype = k2v(dtype_entry->value_string);
     ln_op_check_param_satisfy_msg(LN_ERROR,
                                   dtype != -1,
                                   "\"dtype\" param should be a supported tl_dtype");

     dims_entry = ln_param_table_find_by_arg_name(op_arg->params, "dims");
     ln_op_check_param_exist(LN_ERROR, dims_entry, "dims");
     ln_op_check_param_type(LN_ERROR, dims_entry, LN_PARAM_ARRAY_NUMBER);
     for (i = 0; i < dtype_entry->array_len; i++)
          ln_op_check_param_satisfy_msg(LN_ERROR,
                                        dims_entry->value_array_int[i] > 0,
                                        "\"dims\" array elements should be positive");

     /* allocate tensor memory in need */
     dst_entry->tensor = tl_tensor_zeros(dims_entry->array_len,
                                         dims_entry->value_array_int,
                                         dtype);

     op_arg->priv = dst_entry->tensor;
}

/*
 * Normally we should only do the calculations here. Operations with memory
 * and such should go in pre_run().
 */
static void zeros_run(ln_op_arg *op_arg, ln_error **error)
{
     tl_tensor *dst;

     /* do the real work */
     dst = op_arg->priv;
     memset(dst->data, 0, dst->len*tl_size_of(dst->dtype));
}

/*
 * This function should free all tensor memory pre_run() allocated.
 */
static void zeros_post_run(ln_op_arg *op_arg, ln_error **error)
{
     /* free the tensor memory allocated in pre_run() and run() */
     tl_tensor_free_data_too(op_arg->priv);
}

static ln_op_arg op_arg_zeros = {
     .optype = "zeros",
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_zeros = {
     .op_arg = &op_arg_zeros,
     .pre_run = zeros_pre_run,
     .run = zeros_run,
     .post_run = zeros_post_run
};

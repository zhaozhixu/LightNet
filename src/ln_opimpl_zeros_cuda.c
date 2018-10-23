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
 * This function should do the parameter checking and tensor shape inference.
 */
static void zeros_cuda_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     char *dst_name;
     ln_tensor_entry *dst_entry;
     tl_tensor *dst_tensor;
     ln_param_entry *dtype_entry, *dims_entry;
     int tensors_n, params_n;
     int dtype, i;

     /* check tensors and parameters */
     tensors_n = ln_tensor_list_length(op_arg->tensors_in);
     ln_op_check_tensor_in_len_eq(tensors_n, 0);

     tensors_n = ln_tensor_list_length(op_arg->tensors_out);
     ln_op_check_tensor_out_len_eq(tensors_n, 1);

     dst_name = ln_tensor_list_find_name(op_arg->tensors_out, "dst");
     ln_op_check_tensor_out_exist(dst_name, "dst");
     dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
     ln_op_check_tensor_not_defined(dst_entry, dst_name);

     params_n = ln_param_list_length(op_arg->params);
     ln_op_check_param_len_eq(params_n, 2);

     dtype_entry = ln_param_list_find(op_arg->params, "dtype");
     ln_op_check_param_exist(dtype_entry, "dtype");
     ln_op_check_param_type(dtype_entry, LN_PARAM_STRING);

     dtype = k2v(dtype_entry->value_string);
     ln_op_check_param_satisfy_msg(dtype != -1,
                                   "\"dtype\" param should be a supported tl_dtype");

     dims_entry = ln_param_list_find(op_arg->params, "dims");
     ln_op_check_param_exist(dims_entry, "dims");
     ln_op_check_param_type(dims_entry, LN_PARAM_ARRAY_NUMBER);
     for (i = 0; i < dtype_entry->array_len; i++)
          ln_op_check_param_satisfy_msg(dims_entry->value_array_int[i] > 0,
                                        "\"dims\" array elements should be positive");

     /* define output tensor shape, tensor data should be NULL */
     dst_tensor = tl_tensor_create(NULL, dims_entry->array_len,
                                   dims_entry->value_array_int, dtype);
     dst_entry = ln_tensor_entry_create(dst_name, dst_tensor);
     ln_tensor_table_insert(op_arg->tensor_table, dst_name, dst_entry);

     op_arg->priv = dst_entry;
}

/*
 * This function should only do the calculations.
 */
static void zeros_cuda_run(ln_op_arg *op_arg, ln_error **error)
{
     tl_tensor *dst;

     dst = ((ln_tensor_entry *)op_arg->priv)->tensor;
     ln_memset_cuda(dst->data, 0, dst->len*tl_size_of(dst->dtype));
}

/*
 * This function should undo everything done by pre_run().
 */
static void zeros_cuda_post_run(ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_table_remove(op_arg->tensor_table,
                            ((ln_tensor_entry *)op_arg->priv)->name);
}

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_zeros_cuda = {
     .optype = "zeros_cuda",
     .mtype_in = LN_MEM_CUDA,
     .mtype_out = LN_MEM_CUDA,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_zeros_cuda = {
     .op_arg = &op_arg_zeros_cuda,
     .pre_run = zeros_cuda_pre_run,
     .static_run = NULL,
     .run = zeros_cuda_run,
     .post_run = zeros_cuda_post_run
};

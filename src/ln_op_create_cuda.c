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

static int compute_length(int ndim, const int *dims)
{
     int i, len;

     for (i = 0, len = 1; i < ndim; i++)
          len *= dims[i];
     return len;
}

/*
 * This function should do the parameter checking and tensor memory allocation.
 */
static void create_cuda_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *dst_entry;
     ln_param_entry *dims_entry, *dtype_entry, *data_entry;
     int tensors_n, params_n, dtype, i;
     void *data;

     /* check tensors and parameters */
     tensors_n = ln_tensor_list_length(op_arg->tensors_in);
     ln_op_check_tensor_in_len_eq(LN_ERROR, tensors_n, 0);

     tensors_n = ln_tensor_list_length(op_arg->tensors_out);
     ln_op_check_tensor_out_len_eq(LN_ERROR, tensors_n, 1);

     dst_entry = ln_tensor_list_find_name(op_arg->tensors_out, "dst");
     ln_op_check_tensor_out_exist(LN_ERROR, dst_entry, "dst");
     ln_op_check_tensor_not_defined(LN_ERROR, dst_entry);

     params_n = ln_param_list_length(op_arg->params);
     ln_op_check_param_len_eq(LN_ERROR, params_n, 3);

     dtype_entry = ln_param_list_find(op_arg->params, "dtype");
     ln_op_check_param_exist(LN_ERROR, dtype_entry, "dtype");
     ln_op_check_param_type(LN_ERROR, dtype_entry, LN_PARAM_STRING);

     dtype = k2v(dtype_entry->value_string);
     ln_op_check_param_satisfy_msg(LN_ERROR,
                                   dtype != -1,
                                   "\"dtype\" param should be a supported tl_dtype");

     dims_entry = ln_param_list_find(op_arg->params, "dims");
     ln_op_check_param_exist(LN_ERROR, dims_entry, "dims");
     ln_op_check_param_type(LN_ERROR, dims_entry, LN_PARAM_ARRAY_NUMBER);
     for (i = 0; i < dtype_entry->array_len; i++)
          ln_op_check_param_satisfy_msg(LN_ERROR,
                                        dims_entry->value_array_int[i] > 0,
                                        "\"dims\" array elements should be positive");

     data_entry = ln_param_list_find(op_arg->params, "data");
     ln_op_check_param_exist(LN_ERROR, data_entry, "data");
     ln_op_check(LN_ERROR,
                 data_entry->type == LN_PARAM_ARRAY_NUMBER
                 || data_entry->type == LN_PARAM_NULL,
                 "%s: \"%s\"'s \"%s\" param's value should be of type %s or %s, but got a %s",
                 op_arg->optype, op_arg->name, data_entry->arg_name,
                 ln_param_type_name(LN_PARAM_ARRAY_NUMBER),
                 ln_param_type_name(LN_PARAM_NULL),
                 ln_param_type_name(data_entry->type));

     /* allocate memory in need */
     if (data_entry->type == LN_PARAM_ARRAY_NUMBER) {
          ln_op_check_param_satisfy_msg(LN_ERROR,
                                        compute_length(dims_entry->array_len,
                                                       dims_entry->value_array_int)
                                        == data_entry->array_len,
                                        "\"data\" array length should match with \"dims\"");
          data = ln_alloc(tl_size_of(dtype) * data_entry->array_len);
          for (i = 0; i < data_entry->array_len; i++) {
               tl_convert(tl_padd(data, i, tl_size_of(dtype)), dtype,
                          &data_entry->value_array_double[i], TL_DOUBLE);
          }
          dst_entry->tensor = tl_tensor_create_cuda(data, dims_entry->array_len,
                                                    dims_entry->value_array_int,
                                                    dtype);
     }
     if (data_entry->type == LN_PARAM_NULL)
          dst_entry->tensor = tl_tensor_create_cuda(NULL, dims_entry->array_len,
                                                    dims_entry->value_array_int,
                                                    dtype);
     op_arg->priv = dst_entry->tensor;
}

/*
 * Normally we should only do the calculations here. Operations with memory
 * and such should go in pre_run().
 */
static void create_cuda_run(ln_op_arg *op_arg, ln_error **error)
{

     /* Get tensors and parameters */
     /* ...... */

     /* do the real work */
     /* ...... */
}

/*
 * This function should free all tensor memory pre_run() allocated.
 */
static void create_cuda_post_run(ln_op_arg *op_arg, ln_error **error)
{

     /* free memory allocated in pre_run() */
     tl_tensor_free_data_too_cuda(op_arg->priv);
}

static ln_op_arg op_arg_create_cuda = {
     .optype = "create_cuda",
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_create_cuda = {
     .op_arg = &op_arg_create_cuda,
     .pre_run = create_cuda_pre_run,
     .run = create_cuda_run,
     .post_run = create_cuda_post_run
};

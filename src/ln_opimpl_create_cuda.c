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

static int compute_length(int ndim, const int *dims)
{
     int i, len;

     for (i = 0, len = 1; i < ndim; i++)
          len *= dims[i];
     return len;
}

struct priv_s {
     tl_tensor      *dst;
     char           *dst_name;
     ln_param_entry *data_entry;
     int             dtype;
};

/*
 * This function should do the parameter checking and tensor shape inference.
 */
static void create_cuda_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     char *dst_name;
     ln_tensor_entry *dst_entry;
     tl_tensor *dst_tensor;
     ln_param_entry *dims_entry, *dtype_entry, *data_entry;
     int tensors_n, params_n, dtype, i;
     struct priv_s *priv;

     /* check tensors and parameters */
     tensors_n = ln_tensor_list_length(op_arg->tensors_in);
     ln_opck_tensors_in_len_eq(tensors_n, 0);

     tensors_n = ln_tensor_list_length(op_arg->tensors_out);
     ln_opck_tensors_out_len_eq(tensors_n, 1);

     dst_name = ln_tensor_list_find_name(op_arg->tensors_out, "dst");
     ln_opck_tensor_out_exist(dst_name, "dst");
     dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
     ln_opck_tensor_not_defined(dst_entry, dst_name);

     params_n = ln_param_list_length(op_arg->params);
     ln_opck_params_len_eq(params_n, 3);

     dtype_entry = ln_param_list_find(op_arg->params, "dtype");
     ln_opck_param_exist(dtype_entry, "dtype");
     ln_opck_param_type(dtype_entry, LN_PARAM_STRING);

     dtype = tl_dtype_from_str(dtype_entry->value_string);
     ln_opck_param_satisfy_msg(dtype != -1,
                                   "`dtype` param should be a supported tl_dtype");

     dims_entry = ln_param_list_find(op_arg->params, "dims");
     ln_opck_param_exist(dims_entry, "dims");
     ln_opck_param_type(dims_entry, LN_PARAM_ARRAY_NUMBER);
     for (i = 0; i < dims_entry->array_len; i++)
          ln_opck_param_satisfy_msg(dims_entry->value_array_int[i] > 0,
                                        "`dims` array elements should be positive");

     data_entry = ln_param_list_find(op_arg->params, "data");
     ln_opck_param_exist(data_entry, "data");
     ln_opck(LN_ERROR, data_entry->type == LN_PARAM_ARRAY_NUMBER
                 || data_entry->type == LN_PARAM_NULL,
                 "%s: `%s`'s `%s` param's value should be of type %s or %s, but gets a %s",
                 op_arg->optype, op_arg->name, data_entry->arg_name,
                 ln_param_type_name(LN_PARAM_ARRAY_NUMBER),
                 ln_param_type_name(LN_PARAM_NULL),
                 ln_param_type_name(data_entry->type));

     if (data_entry->type == LN_PARAM_ARRAY_NUMBER) {
          ln_opck_param_satisfy_msg(compute_length(dims_entry->array_len,
                                                       dims_entry->value_array_int)
                                        == data_entry->array_len,
                                        "`data` array length should match with `dims`");
     }

     /* define output tensor shape, tensor data should be NULL */
     dst_tensor = tl_tensor_create(NULL, dims_entry->array_len,
                                   dims_entry->value_array_int,
                                   dtype);
     dst_entry = ln_tensor_entry_create(dst_name, dst_tensor);
     dst_entry->mtype = LN_MEM_CUDA;
     ln_tensor_table_insert(op_arg->tensor_table, dst_name, dst_entry);

     /* set dst static if data is given */
     if (data_entry->type == LN_PARAM_ARRAY_NUMBER) {
          dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
          dst_entry->isstatic = 1;
     }

     /* use op_arg->priv to store private data to be used in other functions */
     priv = ln_alloc(sizeof(struct priv_s));
     priv->dst = dst_tensor;
     priv->dst_name = dst_name;
     priv->data_entry = data_entry;
     priv->dtype = dtype;
     op_arg->priv = priv;
}

/* This function runs only once per instance right after memory allocation. */
static void create_cuda_static_run(ln_op_arg *op_arg, ln_error **error)
{
     struct priv_s *priv;
     ln_param_entry *data_entry;
     int dtype;
     size_t size;
     void *data;

     priv = op_arg->priv;
     data_entry = priv->data_entry;
     if (data_entry->type == LN_PARAM_NULL) /* not static */
          return;

     dtype = priv->dtype;
     size = tl_size_of(dtype) * data_entry->array_len;
     data = ln_alloc(size);
     for (int i = 0; i < data_entry->array_len; i++) {
          tl_convert(tl_padd(data, i, tl_size_of(dtype)), dtype,
                     &data_entry->value_array_double[i], TL_DOUBLE);
     }
     ln_memcpy_h2d(priv->dst->data, data, size);
     ln_free(data);
}

/*
 * This function should undo everything done by pre_run().
 */
static void create_cuda_post_run(ln_op_arg *op_arg, ln_error **error)
{
     struct priv_s *priv;

     priv = op_arg->priv;
     ln_tensor_table_remove(op_arg->tensor_table, priv->dst_name);
     ln_free(op_arg->priv);
}

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_create_cuda = {
     .optype = "create_cuda",
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_create_cuda = {
     .op_arg = &op_arg_create_cuda,
     .pre_run = create_cuda_pre_run,
     .static_run = create_cuda_static_run,
     .run = NULL,
     .post_run = create_cuda_post_run
};

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

static void check_param(char *name1, char *name2, ln_param_type ptype,
                        int plen ,ln_op_arg *op_arg, ln_error **error)
{
     char *full_name;
     ln_param_entry *pe;

     if (name2)
          full_name = ln_strcat_delim_alloc(name1, name2, '_');
     else
          full_name = ln_strdup(name1);
     pe = ln_param_list_find(op_arg->params, full_name);
     ln_opck_param_exist(pe, full_name);
     ln_opck_param_type(pe, ptype);
     if (plen > 0)
          ln_opck_param_array_len_eq(pe, plen);

     ln_free(full_name);
}

/* TODO: check params throughly */
static void check_conv(char *opname, ln_op_arg *op_arg, ln_error **error)
{
     check_param(opname, "src", LN_PARAM_STRING, 0, op_arg, error);
     check_param(opname, "weight", LN_PARAM_STRING, 0, op_arg, error);
     check_param(opname, "dst", LN_PARAM_STRING, 0, op_arg, error);
     check_param(opname, "group", LN_PARAM_NUMBER, 0, op_arg, error);
     check_param(opname, "size", LN_PARAM_ARRAY_NUMBER, 2, op_arg, error);
     check_param(opname, "stride", LN_PARAM_ARRAY_NUMBER, 2, op_arg, error);
     check_param(opname, "padding", LN_PARAM_ARRAY_NUMBER, 2, op_arg, error);
     check_param(opname, "dilation", LN_PARAM_ARRAY_NUMBER, 2, op_arg, error);
}

static void check_activation(char *opname, ln_op_arg *op_arg, ln_error **error)
{
     check_param(opname, "src", LN_PARAM_STRING, 0, op_arg, error);
     check_param(opname, "dst", LN_PARAM_STRING, 0, op_arg, error);
     check_param(opname, "activation_type", LN_PARAM_STRING, 0, op_arg, error);
}

static void check_maxpool2d(char *opname, ln_op_arg *op_arg, ln_error **error)
{
     check_param(opname, "src", LN_PARAM_STRING, 0, op_arg, error);
     check_param(opname, "dst", LN_PARAM_STRING, 0, op_arg, error);
     check_param(opname, "size", LN_PARAM_ARRAY_NUMBER, 2, op_arg, error);
     check_param(opname, "stride", LN_PARAM_ARRAY_NUMBER, 2, op_arg, error);
     check_param(opname, "padding", LN_PARAM_ARRAY_NUMBER, 2, op_arg, error);
}

/*
 * This function should do the parameter checking and tensor shape inference.
 */
static void tensorrt_cuda_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     int tensors_n;
     ln_param_entry *pe;
     ln_tensor_list_entry *tle;
     ln_tensor_entry *te;

     /* check tensors and parameters */
     tensors_n = ln_tensor_list_length(op_arg->tensors_in);
     ln_opck_tensor_in_len_gt(tensors_n, 0);

     LN_LIST_FOREACH(tle, op_arg->tensors_in) {
          if (!strncmp(tle->arg_name, "src", 3)) {
               te = ln_tensor_table_find(op_arg->tensor_table, tle->name);
               ln_opck_tensor_defined(te, tle->name);
               ln_opck_tensor_mtype_eq(te, LN_MEM_CUDA);
          } else if (!strncmp(tle->arg_name, "weight", 6)) {
               te = ln_tensor_table_find(op_arg->tensor_table, tle->name);
               ln_opck_tensor_defined(te, tle->name);
               ln_opck_tensor_mtype_eq(te, LN_MEM_CPU);
               ln_opck_tensor_isstatic(te);
          }
     }

     tensors_n = ln_tensor_list_length(op_arg->tensors_out);
     ln_opck_tensor_out_len_gt(tensors_n, 0);

     LN_LIST_FOREACH(tle, op_arg->tensors_out) {
          if (!strncmp(tle->arg_name, "dst", 3)) {
               te = ln_tensor_table_find(op_arg->tensor_table, tle->name);
               ln_opck_tensor_not_defined(te, tle->name);
          }
     }

     LN_LIST_FOREACH(pe, op_arg->params) {
          if (ln_next_token(pe->arg_name, '_'))
               continue;
          ln_opck_param_type(pe, LN_PARAM_STRING);
          if (!strcmp(pe->value_string, "conv"))
               check_conv(pe->arg_name, op_arg, error);
          else if (!strcmp(pe->value_string, "activation"))
               check_activation(pe->arg_name, op_arg, error);
          else if (!strcmp(pe->value_string, "maxpool2d"))
               check_maxpool2d(pe->arg_name, op_arg, error);
          else
               ln_opck_param_error(0, "unsupported TensorRT operator");
     }

     /* define output tensor shape, tensor data should be NULL */
     ln_tensor_entry *dst_entry;
     tl_tensor *dst_tensor;
     int *dims;
     int ndim;
     tl_dtype dtype;
     char *arg_name;
     LN_LIST_FOREACH(tle, op_arg->tensors_out) {
          arg_name = ln_strcat_delim_alloc(tle->arg_name, "shape", '_');
          pe = ln_param_list_find(op_arg->params, arg_name);
          ln_opck_param_exist(pe, arg_name);
          ln_opck_param_type(pe, LN_PARAM_ARRAY_NUMBER);
          ln_opck_param_array_len_gt(pe, 0);
          dims = pe->value_array_int;
          ndim = pe->array_len;
          ln_free(arg_name);

          arg_name = ln_strcat_delim_alloc(tle->arg_name, "dtype", '_');
          pe = ln_param_list_find(op_arg->params, arg_name);
          ln_opck_param_exist(pe, arg_name);
          ln_opck_param_type(pe, LN_PARAM_ARRAY_STRING);
          dtype = tl_dtype_from_str(pe->value_string);
          ln_free(arg_name);

          dst_tensor = tl_tensor_create(NULL, ndim, dims, dtype);
          dst_entry = ln_tensor_entry_create(tle->name, dst_tensor);
          dst_entry->mtype = LN_MEM_CUDA;
          ln_tensor_table_insert(op_arg->tensor_table, tle->name, dst_entry);
     }

     /* use op_arg->priv to store private data to be used in other functions */
}

/*
 * This function should only do the calculations.
 */
static void tensorrt_cuda_run(ln_op_arg *op_arg, ln_error **error)
{

}

/*
 * This function should undo everything done by pre_run().
 */
static void tensorrt_cuda_post_run(ln_op_arg *op_arg, ln_error **error)
{

}

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_tensorrt_cuda = {
     .optype = "tensorrt_cuda",
     .mtype_in = LN_MEM_CUDA,
     .mtype_out = LN_MEM_CUDA,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_tensorrt_cuda = {
     .op_arg = &op_arg_tensorrt_cuda,
     .pre_run = tensorrt_cuda_pre_run,
     .static_run = NULL,
     .run = tensorrt_cuda_run,
     .post_run = tensorrt_cuda_post_run
};

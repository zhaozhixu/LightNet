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
 * This function should do the parameter checking and tensor shape inference.
 */
static void tensorrt_cuda_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     char *src_name, *dst_name, *arg_name;
     ln_tensor_entry *src_entry, *dst_entry, *arg_entry;
     tl_tensor *dst_tensor, *arg_tensor;
     ln_param_entry *axis_entry;
     int tensors_n, params_n;
     int axis;
     struct priv_s *priv;

     /* check tensors and parameters */
     tensors_n = ln_tensor_list_length(op_arg->tensors_in);
     ln_op_check_tensor_in_len_gt(tensors_n, 0);

     tensors_n = ln_tensor_list_length(op_arg->tensors_out);
     ln_op_check_tensor_out_len_gt(tensors_n, 0);

     src_name = ln_tensor_list_find_name(op_arg->tensors_in, "src");
     ln_op_check_tensor_in_exist(src_name, "src");
     src_entry = ln_tensor_table_find(op_arg->tensor_table, src_name);
     ln_op_check_tensor_defined(src_entry, src_name);

     dst_name = ln_tensor_list_find_name(op_arg->tensors_out, "dst");
     ln_op_check_tensor_out_exist(dst_name, "dst");
     dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
     ln_op_check_tensor_not_defined(dst_entry, dst_name);

     /* "arg" is an optional parameter */
     arg_name = ln_tensor_list_find_name(op_arg->tensors_out, "arg");
     if (arg_name) {
          arg_entry = ln_tensor_table_find(op_arg->tensor_table, arg_name);
          ln_op_check_tensor_not_defined(arg_entry, arg_name);
     }

     params_n = ln_param_list_length(op_arg->params);
     ln_op_check_param_len_eq(params_n, 1);

     /* define output tensor shape, tensor data should be NULL */

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

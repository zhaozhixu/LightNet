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
#include "ln_tensorrt.h"

struct priv_s {
    ln_tensorrt_bundle *bundle;
};

/*
 * This function should do the parameter checking and tensor shape inference.
 */
static void tensorrt_pre_run(ln_op_arg *op_arg, ln_error **error)
{
    /* check tensors and parameters */
    ln_tensorrt_check_op(op_arg, error);

    /* define output tensor shape, tensor data should be NULL */
    ln_param_entry *pe;
    ln_tensor_list_entry *tle;
    ln_tensor_entry *dst_entry;
    tl_tensor *dst_tensor;
    int *dims;
    int ndim;
    tl_dtype dtype;
    char *arg_name;
    LN_LIST_FOREACH(tle, op_arg->tensors_out) {
        arg_name = ln_strcat_delim_alloc(tle->arg_name, "shape", '_');
        pe = ln_param_list_find(op_arg->params, arg_name);
        dims = pe->value_array_int;
        ndim = pe->array_len;
        ln_free(arg_name);

        arg_name = ln_strcat_delim_alloc(tle->arg_name, "dtype", '_');
        pe = ln_param_list_find(op_arg->params, arg_name);
        dtype = tl_dtype_from_str(pe->value_string);
        ln_free(arg_name);

        dst_tensor = tl_tensor_create(NULL, ndim, dims, dtype);
        dst_entry = ln_tensor_entry_create(tle->name, dst_tensor);
        ln_tensor_entry_set_creater(dst_entry, op_arg->name);
        dst_entry->mtype = LN_MEM_CUDA;
        ln_tensor_table_insert(op_arg->tensor_table, dst_entry);
    }

    struct priv_s *priv;
    priv = ln_alloc(sizeof(struct priv_s));
    priv->bundle = NULL;
    op_arg->priv = priv;
}

static void tensorrt_static_run(ln_op_arg *op_arg, ln_error **error)
{
    struct priv_s *priv;

    priv = op_arg->priv;
    priv->bundle = ln_tensorrt_bundle_create(op_arg);
}

/*
 * This function should only do the calculations.
 */
static void tensorrt_run(ln_op_arg *op_arg, ln_error **error)
{
    struct priv_s *priv;

    priv = op_arg->priv;
    ln_tensorrt_bundle_execute(priv->bundle);
}

/*
 * This function should free all the memory allocated by other *_run()s.
 */
static void tensorrt_post_run(ln_op_arg *op_arg, ln_error **error)
{
    struct priv_s *priv;
    ln_tensor_list_entry *tle;

    LN_LIST_FOREACH(tle, op_arg->tensors_out) {
        ln_tensor_table_remove(op_arg->tensor_table, tle->name);
    }
    priv = op_arg->priv;
    ln_tensorrt_bundle_free(priv->bundle);
    ln_free(op_arg->priv);
}

static const char *in_arg_names[] = {
    NULL
};

static const char *out_arg_names[] = {
    NULL
};

static const char *param_arg_names[] = {
    NULL
};

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_tensorrt = {
    .optype = "tensorrt",
    .arch = "tensorrt",
    .in_arg_names = in_arg_names,
    .out_arg_names = out_arg_names,
    .param_arg_names = param_arg_names,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_tensorrt = {
    .op_arg = &op_arg_tensorrt,
    .pre_run = tensorrt_pre_run,
    .static_run = tensorrt_static_run,
    .run = tensorrt_run,
    .post_run = tensorrt_post_run
};

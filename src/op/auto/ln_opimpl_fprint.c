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
#include "ln_arch.h"

struct priv_s {
    ln_tensor_entry *src_entry;
    ln_param_entry  *msg_entry;
    ln_param_entry  *file_entry;
};

/* This function should do the parameter checking and tensor shape inference. */
static void fprint_pre_run(ln_op_arg *op_arg)
{
    char                 *src_name;
    ln_tensor_list_entry *src_list_entry;
    ln_tensor_entry      *src_entry;
    tl_tensor            *src;
    char                 *msg;
    ln_param_entry       *msg_entry;
    char                 *file;
    ln_param_entry       *file_entry;
    int                   tensors_in_n;
    int                   tensors_out_n;
    int                   params_n;
    struct priv_s        *priv;

    /* check tensors and parameters */
    tensors_in_n = ln_tensor_list_length(op_arg->tensors_in);
    ln_opck_tensors_in_len_eq(tensors_in_n, 1);

    src_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "src");
    ln_opck_tensor_in_exist(src_list_entry, "src");
    src_name = src_list_entry->name;
    src_entry = ln_tensor_table_find(op_arg->tensor_table, src_name);
    ln_opck_tensor_defined(src_entry, src_name);
    src = src_entry->tensor;
    src = src;

    tensors_out_n = ln_tensor_list_length(op_arg->tensors_out);
    ln_opck_tensors_out_len_eq(tensors_out_n, 0);

    params_n = ln_param_list_length(op_arg->params);
    ln_opck_params_len_eq(params_n, 2);

    msg_entry = ln_param_list_find(op_arg->params, "msg");
    ln_opck_param_exist(msg_entry, "msg");
    ln_opck_param_type(msg_entry, LN_PARAM_STRING);
    msg = msg_entry->value_string;
    msg = msg;

    file_entry = ln_param_list_find(op_arg->params, "file");
    ln_opck_param_exist(file_entry, "file");
    ln_opck_param_type(file_entry, LN_PARAM_STRING);
    file = file_entry->value_string;
    file = file;

    /* define output tensor shape, tensor data should be NULL */

    /* use op_arg->priv to store private data to be used in other functions */
    priv = ln_alloc(sizeof(struct priv_s));
    priv->src_entry = src_entry;
    priv->msg_entry = msg_entry;
    priv->file_entry = file_entry;
    op_arg->priv = priv;
}

/* This function should free all the memory allocated by other *_run()s. */
static void fprint_post_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;

    ln_free(priv);
}

static const char *in_arg_names[] = {
    "src",
    NULL
};

static const char *out_arg_names[] = {
    NULL
};

static const char *param_arg_names[] = {
    "msg",
    "file",
    NULL
};

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_fprint = {
    .optype = "fprint",
    .arch = "none",
    .in_arg_names = in_arg_names,
    .out_arg_names = out_arg_names,
    .param_arg_names = param_arg_names,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_fprint = {
    .op_arg = &op_arg_fprint,
    .pre_run = fprint_pre_run,
    .static_run = NULL,
    .run = NULL,
    .post_run = fprint_post_run
};

/*
 * Copyright (c) 2018-2019 Zhao Zhixu
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
    ln_tensor_entry *dst_entry;
    ln_param_entry  *dtype_entry;
    ln_param_entry  *dims_entry;
    ln_param_entry  *ran_entry;
    ln_param_entry  *data_entry;
    ln_param_entry  *from_file_entry;
};

/* This function should do the parameter checking and tensor shape inference. */
static void create_cpu_pre_run(ln_op_arg *op_arg)
{
    char                 *dst_name;
    ln_tensor_list_entry *dst_list_entry;
    ln_tensor_entry      *dst_entry;
    tl_tensor            *dst;
    int                   dst_ndim;
    int                  *dst_dims;
    tl_dtype              dst_dtype;
    int                   dtype;
    ln_param_entry       *dtype_entry;
    int                  *dims;
    ln_param_entry       *dims_entry;
    double               *ran;
    ln_param_entry       *ran_entry;
    double               *data;
    ln_param_entry       *data_entry;
    tl_bool_t             from_file;
    ln_param_entry       *from_file_entry;
    int                   tensors_in_n;
    int                   tensors_out_n;
    int                   params_n;
    struct priv_s        *priv;

    /* check tensors and parameters */
    tensors_in_n = ln_tensor_list_length(op_arg->tensors_in);
    ln_opck_tensors_in_len_eq(tensors_in_n, 0);

    tensors_out_n = ln_tensor_list_length(op_arg->tensors_out);
    ln_opck_tensors_out_len_eq(tensors_out_n, 1);

    dst_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "dst");
    ln_opck_tensor_out_exist(dst_list_entry, "dst");
    dst_name = dst_list_entry->name;
    dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
    ln_opck_tensor_not_defined(dst_entry, dst_name);

    params_n = ln_param_list_length(op_arg->params);
    ln_opck_params_len_eq(params_n, 5);

    dtype_entry = ln_param_list_find(op_arg->params, "dtype");
    ln_opck_param_exist(dtype_entry, "dtype");
    ln_opck_param_type(dtype_entry, LN_PARAM_STRING);
    dtype = tl_dtype_from_str(dtype_entry->value_string);
    dtype_entry->value_int = dtype;
    dtype = dtype;
    ln_opck_satisfy_msg(dtype != -1, "`dtype` param should be a supported tl_dtype");

    dims_entry = ln_param_list_find(op_arg->params, "dims");
    ln_opck_param_exist(dims_entry, "dims");
    ln_opck_param_type(dims_entry, LN_PARAM_ARRAY_NUMBER);
    dims = dims_entry->value_array_int;
    ln_opck_param_array_int_gt(dims_entry, 0);
    dims = dims;

    ran_entry = ln_param_list_find(op_arg->params, "ran");
    ln_opck_param_exist(ran_entry, "ran");
    ln_opck_param_type(ran_entry, LN_PARAM_ARRAY_NUMBER);
    ln_opck_param_array_len_eq(ran_entry, 2);
    ran = ran_entry->value_array_double;
    ran = ran;
    ln_opck_satisfy(ran[0] <= ran[1]);

    data_entry = ln_param_list_find(op_arg->params, "data");
    ln_opck_param_exist(data_entry, "data");
    ln_opck_param_type(data_entry, LN_PARAM_ARRAY_NUMBER);
    data = data_entry->value_array_double;
    data = data;

    from_file_entry = ln_param_list_find(op_arg->params, "from_file");
    ln_opck_param_exist(from_file_entry, "from_file");
    ln_opck_param_type(from_file_entry, LN_PARAM_BOOL);
    from_file = from_file_entry->value_bool;
    from_file = from_file;

    /* define output tensor shape, tensor data should be NULL */
    dst_ndim = dims_entry->array_len;
    dst_dims = dims;
    dst_dtype = dtype;
    dst = tl_tensor_create(NULL, dst_ndim, dst_dims, dst_dtype);
    dst_entry = ln_tensor_entry_create(dst_name, dst);
    dst_entry->offset = dst_list_entry->offset;
    ln_tensor_entry_set_creater(dst_entry, op_arg->name);
    dst_entry->isstatic = 1;
    dst_entry->mtype = LN_MEM_CPU;
    ln_tensor_table_insert(op_arg->tensor_table, dst_entry);

    /* use op_arg->priv to store private data to be used in other functions */
    priv = ln_alloc(sizeof(struct priv_s));
    priv->dst_entry = dst_entry;
    priv->dtype_entry = dtype_entry;
    priv->dims_entry = dims_entry;
    priv->ran_entry = ran_entry;
    priv->data_entry = data_entry;
    priv->from_file_entry = from_file_entry;
    op_arg->priv = priv;
}

/* This function runs only once per instance right after memory allocation. */
static void create_cpu_static_run(ln_op_arg *op_arg)
{
    struct priv_s  *priv = op_arg->priv;
    tl_tensor      *dst = priv->dst_entry->tensor;
    int             dtype = priv->dtype_entry->value_int;
    double         *ran = priv->ran_entry->value_array_double;
    ln_param_entry *data_entry = priv->data_entry;
    double         *data = priv->data_entry->value_array_double;
    tl_bool_t       from_file = priv->from_file_entry->value_bool;

    {
        if (from_file)
            return;
        size_t size = tl_tensor_size(dst);
        void *data_tmp = ln_alloc(size);
        if (!(ran[0] == 0 && ran[1] == 0)) {
            srand(time(NULL));
            for (int i = 0; i < dst->len; i++) {
                double r = rand() * (ran[1] - ran[0]) / (double)RAND_MAX + ran[0];
                tl_convert(tl_padd(data_tmp, i, tl_size_of(dtype)), dtype, &r, TL_DOUBLE);
            }
        } else if (data[0] == 0 && data_entry->array_len == 1) {
            memset(data_tmp, 0, size);
        } else {
            for (int i = 0; i < data_entry->array_len; i++)
                tl_convert(tl_padd(data_tmp, i, tl_size_of(dtype)), dtype, &data[i], TL_DOUBLE);
        }
        memmove(dst->data, data_tmp, size);
        ln_free(data_tmp);
    }
}

/* This function should free all the memory allocated by other *_run()s. */
static void create_cpu_post_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;

    ln_tensor_table_remove(op_arg->tensor_table, priv->dst_entry->name);
    ln_free(priv);
}

static const char *in_arg_names[] = {
    NULL
};

static const char *out_arg_names[] = {
    "dst",
    NULL
};

static const char *param_arg_names[] = {
    "dtype",
    "dims",
    "ran",
    "data",
    "from_file",
    NULL
};

static const ln_param_type param_ptypes[] = {
    LN_PARAM_STRING,
    LN_PARAM_ARRAY_NUMBER,
    LN_PARAM_ARRAY_NUMBER,
    LN_PARAM_ARRAY_NUMBER,
    LN_PARAM_BOOL,
};

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_create_cpu = {
    .optype = "create_cpu",
    .arch = "cpu",
    .in_arg_names = in_arg_names,
    .out_arg_names = out_arg_names,
    .param_arg_names = param_arg_names,
    .param_ptypes = param_ptypes,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_create_cpu = {
    .op_arg = &op_arg_create_cpu,
    .pre_run = create_cpu_pre_run,
    .static_run = create_cpu_static_run,
    .run = NULL,
    .post_run = create_cpu_post_run
};

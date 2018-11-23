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

#include <ctype.h>

#include "ln_arch.h"
#include "ln_tensorrt.h"

#define MAX_NAME_SUBFIX 30

extern ln_op ln_opimpl_tensorrt;
/* end of declare tensorrt ops */

static ln_op *ops_tensorrt[] = {
    &ln_opimpl_tensorrt,
    NULL
};

/* TODO: add a private field to ln_arch and put this there? */
static ln_hash *error_hash = NULL;

/* TODO: add a global op name hash */
static ln_op *create_trt_op(ln_op *from_op)
{
    ln_op *trt_op;
    char name[3+MAX_NAME_SUBFIX];

    snprintf(name, 3+MAX_NAME_SUBFIX, "trt%p", from_op);
    trt_op = ln_op_create_from_proto(&ln_opimpl_tensorrt, name, NULL,
                                     NULL, NULL, from_op->op_arg->tensor_table);
    return trt_op;
}

static char *create_name_in_tensors(ln_list *tensors, const char *prefix)
{
    ln_tensor_list_entry *tle;
    int max_idx = 0;
    char *buf;
    size_t prefix_len = strlen(prefix);
    size_t buf_len = prefix_len + MAX_NAME_SUBFIX;

    buf = ln_alloc(sizeof(char)*buf_len);
    LN_LIST_FOREACH(tle, tensors) {
        if (strncmp(tle->arg_name, prefix, prefix_len) ||
            ln_next_token(tle->arg_name, '_'))
            continue;
        assert(isdigit(tle->arg_name[prefix_len]) && "subfixed with no digit");
        int idx = atoi(&tle->arg_name[prefix_len]);
        max_idx = max_idx < idx ? idx : max_idx;
    }
    snprintf(buf, buf_len, "%s%d", prefix, max_idx);
    return buf;
}

static char *create_name_in_params(ln_list *params, const char *prefix)
{
    ln_param_entry *pe;
    int max_idx = 0;
    char *buf;
    size_t prefix_len = strlen(prefix);
    size_t buf_len = prefix_len + MAX_NAME_SUBFIX;

    buf = ln_alloc(sizeof(char)*buf_len);
    LN_LIST_FOREACH(pe, params) {
        if (strncmp(pe->arg_name, prefix, prefix_len) ||
            ln_next_token(pe->arg_name, '_'))
            continue;
        assert(isdigit(pe->arg_name[prefix_len]) && "subfixed with no digit");
        int idx = atoi(&pe->arg_name[prefix_len]);
        max_idx = max_idx < idx ? idx : max_idx;
    }
    snprintf(buf, buf_len, "%s%d", prefix, max_idx);
    return buf;
}

static int exists_in_tensors(ln_list *tensors, const char *name)
{
    ln_tensor_list_entry *tle;

    LN_LIST_FOREACH(tle, tensors) {
        if (ln_streq(tle->name, name))
            return 1;
    }
    return 0;
}

static void add_src(ln_op_arg *trt_arg, ln_op_arg *arg, char *opname,
                    char *origin_tensor_arg)
{
    char *tensor_name;
    char *param_arg_name;
    char *tensor_arg_name;

    tensor_name = ln_tensor_list_find_name(arg->tensors_in, origin_tensor_arg);
    param_arg_name = ln_strcat_delim_alloc(opname, "src", '_');
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  param_arg_name, tensor_name);
    ln_free(param_arg_name);

    if (!exists_in_tensors(trt_arg->tensors_out, tensor_name)) {
        tensor_arg_name = create_name_in_tensors(trt_arg->tensors_in, "src");
        trt_arg->tensors_in = ln_tensor_list_append(trt_arg->tensors_in,
                                                    tensor_arg_name, tensor_name);
        ln_free(tensor_arg_name);
    }
}

static void add_weight(ln_op_arg *trt_arg, ln_op_arg *arg, char *opname,
                       char *origin_tensor_arg)
{
    char *tensor_name;
    char *param_arg_name;
    char *tensor_arg_name;

    tensor_name = ln_tensor_list_find_name(arg->tensors_in, origin_tensor_arg);
    param_arg_name = ln_strcat_delim_alloc(opname, "weight", '_');
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  param_arg_name, tensor_name);
    ln_free(param_arg_name);

    tensor_arg_name = create_name_in_tensors(trt_arg->tensors_in, "weight");
    trt_arg->tensors_in = ln_tensor_list_append(trt_arg->tensors_in,
                                                tensor_arg_name, tensor_name);
    ln_free(tensor_arg_name);
}

static void add_dst(ln_op_arg *trt_arg, ln_op_arg *arg, char *opname,
                    char *origin_tensor_arg)
{
    char *tensor_name;
    char *param_arg_name;
    char *tensor_arg_name;
    ln_tensor_entry *te;

    tensor_name = ln_tensor_list_find_name(arg->tensors_out, origin_tensor_arg);
    param_arg_name = ln_strcat_delim_alloc(opname, "dst", '_');
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  param_arg_name, tensor_name);
    ln_free(param_arg_name);

    tensor_arg_name = create_name_in_tensors(trt_arg->tensors_out, "dst");
    trt_arg->tensors_out = ln_tensor_list_append(trt_arg->tensors_out,
                                                 tensor_arg_name, tensor_name);

    param_arg_name = ln_strcat_delim_alloc(tensor_arg_name, "shape", '_');
    te = ln_tensor_table_find(arg->tensor_table, tensor_name);
    trt_arg->params = ln_param_list_append_array_int(trt_arg->params,
                                                     param_arg_name,
                                                     te->tensor->ndim,
                                                     te->tensor->dims);
    ln_free(param_arg_name);

    param_arg_name = ln_strcat_delim_alloc(tensor_arg_name, "dtype", '_');
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  param_arg_name,
                                                  tl_dtype_name(te->tensor->dtype));

    ln_free(param_arg_name);
    ln_free(tensor_arg_name);
}

static void check_and_add_batch_size(ln_op *trt_op, int batch_size,
                                     const char *opname)
{
    ln_param_entry *pe;

    pe = ln_param_list_find(trt_op->op_arg->params, "batch_size");
    if (!pe)
        trt_op->op_arg->params = ln_param_list_append_number(trt_op->op_arg->params,
                                                             "batch_size", batch_size);
    else if (batch_size != pe->value_int)
        ln_error_emit(LN_ERROR,
                      "batch size doesn't match among ops when converting to TensorRT: original batch_size = %d, '%s''s batch_size = %d",
                      pe->value_int, opname, batch_size);
    else
        return;
}

static int check_conv(ln_op *op)
{
    ln_param_entry *pe;

    pe = ln_param_list_find(op->op_arg->params, "padding");
    if (pe->value_array_int[0] != pe->value_array_int[1]
        || pe->value_array_int[2] != pe->value_array_int[3]) {
        ln_error_emit_once(error_hash, op->op_arg->name, LN_WARNING,
                           "cannot convert '%s' with asymmetrical padding to TensorRT conv op",
                           op->op_arg->name);
        return 0;
    }
    return 1;
}

static void add_conv_to_trt(ln_op *trt_op, ln_op *op)
{
    ln_param_entry *pe;
    ln_tensor_entry *te;
    char *tensor_name;
    ln_op_arg *trt_arg = trt_op->op_arg;
    ln_op_arg *op_arg = op->op_arg;
    char *opname;
    char *param_arg_name;

    opname = create_name_in_params(op_arg->params, "op");
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  opname, "conv");

    add_src(trt_arg, op_arg, opname, "src");
    add_weight(trt_arg, op_arg, opname, "weight");
    add_weight(trt_arg, op_arg, opname, "bias");
    add_dst(trt_arg, op_arg, opname, "dst");

    tensor_name = ln_tensor_list_find_name(op_arg->tensors_out, "src");
    te = ln_tensor_table_find(op_arg->tensor_table, tensor_name);
    check_and_add_batch_size(trt_op, te->tensor->dims[0], op_arg->name);

    pe = ln_param_list_find(op_arg->params, "group");
    param_arg_name = ln_strcat_delim_alloc(opname, "group", '_');
    trt_arg->params = ln_param_list_append_number(trt_arg->params,
                                                  param_arg_name, pe->value_double);
    ln_free(param_arg_name);

    tensor_name = ln_tensor_list_find_name(op_arg->tensors_out, "dst");
    te = ln_tensor_table_find(op_arg->tensor_table, tensor_name);
    param_arg_name = ln_strcat_delim_alloc(opname, "output_c", '_');
    trt_arg->params = ln_param_list_append_number(trt_arg->params,
                                                  param_arg_name, te->tensor->dims[1]);
    ln_free(param_arg_name);

    pe = ln_param_list_find(op_arg->params, "size");
    param_arg_name = ln_strcat_delim_alloc(opname, "size", '_');
    trt_arg->params = ln_param_list_append_array_number(trt_arg->params,
                                                        param_arg_name, 2,
                                                        pe->value_array_double);
    ln_free(param_arg_name);

    pe = ln_param_list_find(op_arg->params, "stride");
    param_arg_name = ln_strcat_delim_alloc(opname, "stride", '_');
    trt_arg->params = ln_param_list_append_array_number(trt_arg->params,
                                                        param_arg_name, 2,
                                                        pe->value_array_double);
    ln_free(param_arg_name);

    pe = ln_param_list_find(op_arg->params, "padding");
    param_arg_name = ln_strcat_delim_alloc(opname, "padding", '_');
    trt_arg->params = ln_param_list_append_array_number(trt_arg->params,
                                                        param_arg_name, 2,
                                                        (double[]){pe->value_array_double[0],
                                                                pe->value_array_double[2]});
    ln_free(param_arg_name);

    pe = ln_param_list_find(op_arg->params, "dilation");
    param_arg_name = ln_strcat_delim_alloc(opname, "dilation", '_');
    trt_arg->params = ln_param_list_append_array_number(trt_arg->params,
                                                        param_arg_name, 2,
                                                        pe->value_array_double);
    ln_free(param_arg_name);

    ln_free(opname);
}

static void add_activation_to_trt(ln_op *trt_op, ln_op *op)
{
    ln_op_arg *trt_arg = trt_op->op_arg;
    ln_op_arg *op_arg = op->op_arg;
    char *opname;
    const char *atype;
    char *param_arg_name;

    opname = create_name_in_params(op_arg->params, "op");
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  opname, "activation");

    add_src(trt_arg, op_arg, opname, "src");
    add_dst(trt_arg, op_arg, opname, "dst");

    if (ln_streq(op_arg->optype, "relu"))
        atype = "kRELU";
    else if (ln_streq(op_arg->optype, "sigmoid"))
        atype = "kSIGMOID";
    else if (ln_streq(op_arg->optype, "tanh"))
        atype = "kTANH";
    else
        assert(0 && "unsupported activation type");

    param_arg_name = ln_strcat_delim_alloc(opname, "activation_type", '_');
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  param_arg_name,
                                                  atype);
    ln_free(param_arg_name);
    ln_free(opname);
}

static int check_pooling(ln_op *op)
{
    ln_param_entry *pe;

    pe = ln_param_list_find(op->op_arg->params, "padding");
    if (pe->value_array_int[0] != pe->value_array_int[1]
        || pe->value_array_int[2] != pe->value_array_int[3]) {
        ln_error_emit_once(error_hash, op->op_arg->name, LN_WARNING,
                           "cannot convert '%s' with asymmetrical padding to TensorRT pooling op",
                           op->op_arg->name);
        return 0;
    }
    return 1;
}

static void add_pooling_to_trt(ln_op *trt_op, ln_op *op)
{
    ln_op_arg *trt_arg = trt_op->op_arg;
    ln_op_arg *op_arg = op->op_arg;
    char *opname;
    char *tensor_name;
    ln_tensor_entry *te;
    const char *pool_type;
    char *param_arg_name;
    ln_param_entry *pe;

    opname = create_name_in_params(op_arg->params, "op");
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  opname, "pooling");

    add_src(trt_arg, op_arg, opname, "src");
    add_dst(trt_arg, op_arg, opname, "dst");

    tensor_name = ln_tensor_list_find_name(op_arg->tensors_out, "src");
    te = ln_tensor_table_find(op_arg->tensor_table, tensor_name);
    check_and_add_batch_size(trt_op, te->tensor->dims[0], op_arg->name);

    if (ln_streq(op_arg->optype, "maxpool2d"))
        pool_type = "kMAX";
    else if (ln_streq(op_arg->optype, "averagepool2d"))
        pool_type = "kAVERAGE";
    else
        assert(0 && "unsupported pooling type");

    param_arg_name = ln_strcat_delim_alloc(opname, "pooling_type", '_');
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  param_arg_name,
                                                  pool_type);
    ln_free(param_arg_name);

    pe = ln_param_list_find(op_arg->params, "size");
    param_arg_name = ln_strcat_delim_alloc(opname, "size", '_');
    trt_arg->params = ln_param_list_append_array_number(trt_arg->params,
                                                        param_arg_name, 2,
                                                        pe->value_array_double);
    ln_free(param_arg_name);

    pe = ln_param_list_find(op_arg->params, "stride");
    param_arg_name = ln_strcat_delim_alloc(opname, "stride", '_');
    trt_arg->params = ln_param_list_append_array_number(trt_arg->params,
                                                        param_arg_name, 2,
                                                        pe->value_array_double);
    ln_free(param_arg_name);

    pe = ln_param_list_find(op_arg->params, "padding");
    param_arg_name = ln_strcat_delim_alloc(opname, "padding", '_');
    trt_arg->params = ln_param_list_append_array_number(trt_arg->params,
                                                        param_arg_name, 2,
                                                        (double[]){pe->value_array_double[0],
                                                                pe->value_array_double[2]});
    ln_free(param_arg_name);

    ln_free(opname);
}

static int check_softmax(ln_op *op)
{
    ln_param_entry *pe;
    ln_tensor_entry *te;
    char *error_key;

    pe = ln_param_list_find(op->op_arg->params, "axis");
    te = ln_tensor_table_find(op->op_arg->tensor_table,
                              ln_tensor_list_find_name(op->op_arg->tensors_in,
                                                       "src"));
    assert(te);

    if (pe->value_int == 0) {
        error_key = ln_strcat_delim_alloc(op->op_arg->name, "eq0", '_');
        ln_error_emit_once(error_hash, error_key, LN_WARNING,
                           "cannot convert '%s' with 'axis' = 0 to TensorRT softmax op",
                           op->op_arg->name);
        ln_free(error_key);
        return 0;
    }
    if (strverscmp(ln_tensorrt_version_str(), "4.0.0") < 0) {
        if (te->tensor->ndim < 4 && pe->value_int != 1) {
            error_key = ln_strcat_delim_alloc(op->op_arg->name, "ne1", '_');
            ln_error_emit_once(error_hash, error_key, LN_WARNING,
                               "cannot convert '%s' with 'src''s number of dimensions < 4 and 'axis' != 1 to TensorRT softmax op in TensorRT version < 4.0.0",
                               op->op_arg->name);
            ln_free(error_key);
            return 0;
        }
        if (te->tensor->ndim >= 4 && pe->value_int != te->tensor->ndim-3) {
            error_key = ln_strcat_delim_alloc(op->op_arg->name, "nendim-3", '_');
            ln_error_emit_once(error_hash, error_key, LN_WARNING,
                               "cannot convert '%s' with 'src''s number of dimensions >= 4 and 'axis' != 'src''s number of dimensions minus 3 to TensorRT softmax op in TensorRT version < 4.0.0",
                               op->op_arg->name);
            ln_free(error_key);
            return 0;
        }
    }
    return 1;
}

static void add_softmax_to_trt(ln_op *trt_op, ln_op *op)
{
    ln_op_arg *trt_arg = trt_op->op_arg;
    ln_op_arg *op_arg = op->op_arg;
    char *opname;
    char *tensor_name;
    ln_tensor_entry *te;
    ln_param_entry *pe;
    int axes = 0;
    char *param_arg_name;

    opname = create_name_in_params(op_arg->params, "op");
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  opname, "softmax");

    add_src(trt_arg, op_arg, opname, "src");
    add_dst(trt_arg, op_arg, opname, "dst");

    tensor_name = ln_tensor_list_find_name(op_arg->tensors_out, "src");
    te = ln_tensor_table_find(op_arg->tensor_table, tensor_name);
    check_and_add_batch_size(trt_op, te->tensor->dims[0], op_arg->name);

    pe = ln_param_list_find(op->op_arg->params, "axis");

    if (strverscmp(ln_tensorrt_version_str(), "4.0.0") >= 0) {
        axes |= 1 << (pe->value_int - 1);
        param_arg_name = ln_strcat_delim_alloc(opname, "axes", '_');
        trt_arg->params = ln_param_list_append_number(trt_arg->params, param_arg_name, axes);
        ln_free(param_arg_name);
    }

    ln_free(opname);
}

static int check_concat(ln_op *op)
{
    ln_param_entry *pe;
    ln_tensor_entry *te;
    char *error_key;

    pe = ln_param_list_find(op->op_arg->params, "axis");
    te = ln_tensor_table_find(op->op_arg->tensor_table,
                              ln_tensor_list_find_name(op->op_arg->tensors_in,
                                                       "src"));
    assert(te);

    if (pe->value_int == 0) {
        error_key = ln_strcat_delim_alloc(op->op_arg->name, "eq0", '_');
        ln_error_emit_once(error_hash, error_key, LN_WARNING,
                           "cannot convert '%s' with 'axis' = 0 to TensorRT concat op",
                           op->op_arg->name);
        ln_free(error_key);
        return 0;
    }
    if (strverscmp(ln_tensorrt_version_str(), "4.0.0") < 0) {
        if (te->tensor->ndim < 4 && pe->value_int != 1) {
            error_key = ln_strcat_delim_alloc(op->op_arg->name, "ne1", '_');
            ln_error_emit_once(error_hash, error_key, LN_WARNING,
                               "cannot convert '%s' with 'src''s number of dimensions < 4 and 'axis' != 1 to TensorRT concat op in TensorRT version < 4.0.0",
                               op->op_arg->name);
            ln_free(error_key);
            return 0;
        }
        if (te->tensor->ndim >= 4 && pe->value_int != te->tensor->ndim-3) {
            error_key = ln_strcat_delim_alloc(op->op_arg->name, "nendim-3", '_');
            ln_error_emit_once(error_hash, error_key, LN_WARNING,
                               "cannot convert '%s' with 'src''s number of dimensions >= 4 and 'axis' != 'src''s number of dimensions minus 3 to TensorRT concat op in TensorRT version < 4.0.0",
                               op->op_arg->name);
            ln_free(error_key);
            return 0;
        }
    }
    return 1;
}

static void add_concat_to_trt(ln_op *trt_op, ln_op *op)
{
    ln_op_arg *trt_arg = trt_op->op_arg;
    ln_op_arg *op_arg = op->op_arg;
    char *opname;
    char *tensor_name;
    ln_tensor_entry *te;
    ln_param_entry *pe;
    int axis;
    char *param_arg_name;

    opname = create_name_in_params(op_arg->params, "op");
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  opname, "concat");

    add_src(trt_arg, op_arg, opname, "src");
    add_dst(trt_arg, op_arg, opname, "dst");

    tensor_name = ln_tensor_list_find_name(op_arg->tensors_out, "src");
    te = ln_tensor_table_find(op_arg->tensor_table, tensor_name);
    check_and_add_batch_size(trt_op, te->tensor->dims[0], op_arg->name);

    pe = ln_param_list_find(op->op_arg->params, "axis");

    if (strverscmp(ln_tensorrt_version_str(), "4.0.0") >= 0) {
        axis = pe->value_int - 1;
        param_arg_name = ln_strcat_delim_alloc(opname, "axis", '_');
        trt_arg->params = ln_param_list_append_number(trt_arg->params,
                                                      param_arg_name, axis);
        ln_free(param_arg_name);
    }

    ln_free(opname);
}

static void add_trt_to_trt(ln_op *trt_op, ln_op *op)
{
}

typedef void (*add_to_trt_func)(ln_op *trt_op, ln_op *op);
static ln_hash_init_entry init_add_funcs[] = {
    {"conv2d", add_conv_to_trt},
    {"relu", add_activation_to_trt},
    {"maxpool2d", add_pooling_to_trt},
    {"softmax", add_softmax_to_trt},
    {"concat", add_concat_to_trt},
    {"tensorrt", add_trt_to_trt},
    LN_HASH_INIT_ENTRY_NULL
};
static ln_hash *add_funcs_hash = NULL;

typedef int (*check_func)(ln_op *op);
static ln_hash_init_entry init_check_funcs[] = {
    {"conv2d", check_conv},
    {"relu", NULL},
    {"maxpool2d", check_pooling},
    {"batchnorm", NULL},
    {"softmax", check_softmax},
    {"concat", NULL},
    {"tensorrt", NULL},
    LN_HASH_INIT_ENTRY_NULL
};
static ln_hash *check_funcs_hash = NULL;

static int is_win_match(ln_list *ops)
{
    ln_op *op;
    ln_list *l;
    void *value;
    check_func check;

    for (l = ops; l; l = l->next) {
        op = l->data;
        if (ln_hash_find_extended(add_funcs_hash, op->op_arg->optype,
                                  NULL, &value)) {
            if (ln_streq(op->op_arg->optype, "tensorrt")) {
                if (l->next) {
                    op = l->next->data;
                    if (ln_streq(op->op_arg->optype, "tensorrt"))
                        return 1;
                }
            } else {
                if (value) {
                    check = value;
                    if (check(op))
                        return 1;
                } else {
                    return 1;
                }
            }
        }
    }
    return 0;
}

static ln_list *ph_func_tensorrt(ln_list *ops, int win_size, int *match)
{
    /* TODO: multithread safety */
    if (!add_funcs_hash) {
        add_funcs_hash = ln_hash_create(ln_str_hash, ln_str_cmp, NULL, NULL);
        ln_hash_init(add_funcs_hash, init_add_funcs);
    }
    if (!check_funcs_hash) {
        check_funcs_hash = ln_hash_create(ln_str_hash, ln_str_cmp, NULL, NULL);
        ln_hash_init(check_funcs_hash, init_check_funcs);
    }
    if (!error_hash)
        error_hash = ln_hash_create(ln_str_hash, ln_str_cmp, NULL, NULL);

    *match = 0;
    if (is_win_match(ops))
        *match = 1;
    else
        return NULL;

    ln_list *new_ops = NULL;
    ln_op *op, *new_op, *prev_op = NULL;
    add_to_trt_func add_func;
    ln_op *trt_op;
    void *value;

    for (ln_list *l = ops; l; prev_op = op, l = l->next) {
        op = l->data;
        if (ln_hash_find_extended(add_funcs_hash, op->op_arg->optype,
                                  NULL, &value)) {
            add_func = value;
            if (prev_op && ln_streq(prev_op->op_arg->optype, "tensorrt")) {
                trt_op = prev_op;
                add_func(trt_op, op);
            } else {
                trt_op = create_trt_op(op);
                add_func(trt_op, op);
                new_ops = ln_list_append(new_ops, trt_op);
            }
            continue;
        }

        new_op = ln_op_create_from_proto(op, op->op_arg->name,
                                         ln_tensor_list_copy(op->op_arg->tensors_in),
                                         ln_tensor_list_copy(op->op_arg->tensors_out),
                                         ln_param_list_copy(op->op_arg->params),
                                         op->op_arg->tensor_table);
        new_ops = ln_list_append(new_ops, new_op);
    }

    return new_ops;
}

static int op_edge_node_cmp_by_edge(void *data1, void *data2)
{
    ln_graph_edge_node *en1 = data1;
    ln_graph_edge_node *en2 = data2;

    return strcmp(en1->edge_data, en2->edge_data);
}

static int is_tensor_been_refered(ln_list *edge_nodes, char *tensor_name)
{
    ln_graph_edge_node en;

    en.edge_data = tensor_name;
    if (ln_list_find_custom(edge_nodes, &en, op_edge_node_cmp_by_edge))
        return 1;
    return 0;
}

static ln_list *post_ph_tensorrt(ln_list *ops)
{
    ln_op *op;
    ln_graph *DFG;
    ln_hash *node_table;
    ln_graph_node *node;
    ln_tensor_list_entry *tle;

    DFG = ln_op_list_gen_DFG(ops, &node_table);
    LN_LIST_FOREACH(op, ops) {
        if (!ln_streq(op->op_arg->optype, "tensorrt"))
            continue;

        node = ln_hash_find(node_table, op->op_arg->name);
        assert(node);
        for (ln_list *l = op->op_arg->tensors_out; l;) {
            tle = l->data;
            if (is_tensor_been_refered(node->edge_nodes, tle->name)) {
                l = l->next;
                continue;
            }
            ln_tensor_table_remove(op->op_arg->tensor_table, tle->name);
            l = l->next;
            ln_list_remove(op->op_arg->tensors_out, tle);
        }
    }
    ln_graph_free(DFG);
    ln_hash_free(node_table);
    return ops;
}

ln_peephole_func ph_funcs_tensorrt[] = {
    ph_func_tensorrt,
    NULL
};

ln_arch ln_arch_tensorrt = {
    .ops = ops_tensorrt,
    .ph_funcs = ph_funcs_tensorrt,
    .post_ph = post_ph_tensorrt,
    .arch_name = "tensorrt",
};

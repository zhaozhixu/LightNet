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
#include "ln_name.h"
#include "ln_tensorrt.h"

extern ln_op ln_opimpl_tensorrt;
/* end of declare tensorrt ops */

static ln_op *ops_tensorrt[] = {
    &ln_opimpl_tensorrt,
/* end of init tensorrt ops */
    NULL
};

static ln_list *ep_create(const ln_op *op, const ln_dfg *dfg, int *match);
static ln_list *ep_conv2d(const ln_op *op, const ln_dfg *dfg, int *match);
static ln_list *ep_relu(const ln_op *op, const ln_dfg *dfg, int *match);
static ln_list *ep_maxpool2d(const ln_op *op, const ln_dfg *dfg, int *match);
static ln_list *ep_softmax(const ln_op *op, const ln_dfg *dfg, int *match);
static ln_list *ep_concat(const ln_op *op, const ln_dfg *dfg, int *match);
static ln_list *ep_batchnorm(const ln_op *op, const ln_dfg *dfg, int *match);
static ln_list *ep_elew(const ln_op *op, const ln_dfg *dfg, int *match);
static ln_list *ep_maxreduce(const ln_op *op, const ln_dfg *dfg, int *match);
static ln_list *ep_maxreduce_arg(const ln_op *op, const ln_dfg *dfg, int *match);
static ln_list *ep_slice(const ln_op *op, const ln_dfg *dfg, int *match);
static ln_list *ep_transpose(const ln_op *op, const ln_dfg *dfg, int *match);
static ln_list *ep_upsample(const ln_op *op, const ln_dfg *dfg, int *match);
static ln_list *ep_zeros(const ln_op *op, const ln_dfg *dfg, int *match);
static ln_list *ep_reshape(const ln_op *op, const ln_dfg *dfg, int *match);
static ln_list *ep_print(const ln_op *op, const ln_dfg *dfg, int *match);
static ln_list *ep_tensorrt(const ln_op *op, const ln_dfg *dfg, int *match);

static ln_hash_init_entry init_ep_funcs[] = {
    {"create", ep_create},
    {"conv2d", ep_conv2d},
    {"relu", ep_relu},
    {"maxpool2d", ep_maxpool2d},
    {"softmax", ep_softmax},
    {"concat", ep_concat},
    {"batchnorm", ep_batchnorm},
    {"elew", ep_elew},
    {"maxreduce", ep_maxreduce},
    {"maxreduce", ep_maxreduce_arg},
    {"slice", ep_slice},
    {"transpose", ep_transpose},
    {"upsample", ep_upsample},
    {"zeros", ep_zeros},
    {"reshape", ep_reshape},
    {"print", ep_print},
    {"tensorrt", ep_tensorrt},
    LN_HASH_INIT_ENTRY_NULL
};
static ln_hash *ep_funcs_hash = NULL;

static ln_list *ep_func_tensorrt(const ln_op *op, const ln_dfg *dfg, int *match);
static ln_list *cb_func_tensorrt(const ln_list *win_ops, size_t win_size,
                                 const ln_dfg *dfg, int *match);
static void init(void);
static void cleanup(void);

ln_expander_func ep_funcs_tensorrt[] = {
    ep_func_tensorrt,
    NULL
};

ln_combiner_func cb_funcs_tensorrt[] = {
    cb_func_tensorrt,
    NULL
};

ln_arch ln_arch_tensorrt = {
    .init_func = init,
    .cleanup_func = cleanup,
    .reg_ops = ops_tensorrt,
    .ep_funcs = ep_funcs_tensorrt,
    .cb_funcs = cb_funcs_tensorrt,
    .arch_name = "tensorrt",
};

static int check_conv(const ln_op *op);
static int check_pooling(const ln_op *op);
static int check_softmax(const ln_op *op);
static int check_concat(const ln_op *op);

static void add_conv_to_trt(ln_op *trt_op, const ln_op *op);
static void add_activation_to_trt(ln_op *trt_op, const ln_op *op);
static void add_pooling_to_trt(ln_op *trt_op, const ln_op *op);
static void add_softmax_to_trt(ln_op *trt_op, const ln_op *op);
static void add_concat_to_trt(ln_op *trt_op, const ln_op *op);
static void add_trt_to_trt(ln_op *trt_op, const ln_op *op, const ln_dfg *dfg);

static void init(void)
{
    ep_funcs_hash = ln_hash_create(ln_str_hash, ln_str_cmp, NULL, NULL);
    ln_hash_init(ep_funcs_hash, init_ep_funcs);
}

static void cleanup(void)
{
    ln_hash_free(ep_funcs_hash);
}

/* TODO: use char[] to replace ln_strcat() etc. */
static char *create_arg_name_in_tensors(ln_list *tensors, const char *prefix)
{
    ln_tensor_list_entry *tle;
    int max_idx = -1;
    char *buf;
    size_t prefix_len = strlen(prefix);
    size_t buf_len = prefix_len + LN_MAX_NAME_SUBFIX;

    buf = ln_alloc(sizeof(char)*buf_len);
    LN_LIST_FOREACH(tle, tensors) {
        if (!ln_streqn(tle->arg_name, prefix, prefix_len) ||
            ln_next_token(tle->arg_name, '_'))
            continue;
        assert(isdigit(tle->arg_name[prefix_len]) && "subfixed with no digit");
        int idx = atoi(&tle->arg_name[prefix_len]);
        max_idx = max_idx < idx ? idx : max_idx;
    }
    snprintf(buf, buf_len, "%s%d", prefix, max_idx+1);
    return buf;
}

static char *create_arg_name_in_params(ln_list *params, const char *prefix)
{
    ln_param_entry *pe;
    int max_idx = -1;
    char *buf;
    size_t prefix_len = strlen(prefix);
    size_t buf_len = prefix_len + LN_MAX_NAME_SUBFIX;

    buf = ln_alloc(sizeof(char)*buf_len);
    LN_LIST_FOREACH(pe, params) {
        if (!ln_streqn(pe->arg_name, prefix, prefix_len) ||
            ln_next_token(pe->arg_name, '_'))
            continue;
        assert(isdigit(pe->arg_name[prefix_len]) && "subfixed with no digit");
        int idx = atoi(&pe->arg_name[prefix_len]);
        max_idx = max_idx < idx ? idx : max_idx;
    }
    snprintf(buf, buf_len, "%s%d", prefix, max_idx+1);
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
                    char *origin_tensor_arg, char *param_arg)
{
    char *tensor_name;
    char *param_arg_name;
    char *tensor_arg_name;

    tensor_name = ln_tensor_list_find_name(arg->tensors_in, origin_tensor_arg);
    param_arg_name = ln_strcat_delim_alloc(opname, param_arg, '_');
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  param_arg_name, tensor_name);
    ln_free(param_arg_name);

    if (!exists_in_tensors(trt_arg->tensors_out, tensor_name) &&
        !exists_in_tensors(trt_arg->tensors_in, tensor_name)) {
        tensor_arg_name = create_arg_name_in_tensors(trt_arg->tensors_in, "src");
        trt_arg->tensors_in = ln_tensor_list_append(trt_arg->tensors_in,
                                                    tensor_arg_name,
                                                    tensor_name);
        ln_free(tensor_arg_name);
    }
}

static void add_weight(ln_op_arg *trt_arg, ln_op_arg *arg, char *opname,
                       char *origin_tensor_arg, char *param_arg)
{
    char *tensor_name;
    char *param_arg_name;
    char *tensor_arg_name;

    tensor_name = ln_tensor_list_find_name(arg->tensors_in, origin_tensor_arg);
    param_arg_name = ln_strcat_delim_alloc(opname, param_arg, '_');
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  param_arg_name, tensor_name);
    ln_free(param_arg_name);

    if (!exists_in_tensors(trt_arg->tensors_in, tensor_name)) {
        tensor_arg_name = create_arg_name_in_tensors(trt_arg->tensors_in,
                                                     "weight");
        trt_arg->tensors_in = ln_tensor_list_append(trt_arg->tensors_in,
                                                    tensor_arg_name,
                                                    tensor_name);
        ln_free(tensor_arg_name);
    }
}

static void add_dst(ln_op_arg *trt_arg, ln_op_arg *arg, char *opname,
                    char *origin_tensor_arg, char *param_arg)
{
    char *tensor_name;
    char *param_arg_name;
    char *tensor_arg_name;
    ln_tensor_entry *te;

    tensor_name = ln_tensor_list_find_name(arg->tensors_out, origin_tensor_arg);
    param_arg_name = ln_strcat_delim_alloc(opname, param_arg, '_');
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  param_arg_name, tensor_name);
    ln_free(param_arg_name);

    if (exists_in_tensors(trt_arg->tensors_out, tensor_name))
        return;

    tensor_arg_name = create_arg_name_in_tensors(trt_arg->tensors_out, "dst");
    trt_arg->tensors_out = ln_tensor_list_append(trt_arg->tensors_out,
                                                 tensor_arg_name,
                                                 tensor_name);

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
        ln_msg_emit(LN_ERROR,
                      "batch size doesn't match among ops when converting to TensorRT: original batch_size = %d, '%s''s batch_size = %d",
                      pe->value_int, opname, batch_size);
    else
        return;
}

static int check_conv(const ln_op *op)
{
    ln_param_entry *pe;

    pe = ln_param_list_find(op->op_arg->params, "padding");
    if (pe->value_array_int[0] != pe->value_array_int[2]
        || pe->value_array_int[1] != pe->value_array_int[3]) {
        ln_msg_emit(LN_WARNING,
                      "cannot convert '%s' with asymmetrical padding to TensorRT conv op",
                      op->op_arg->name);
        return 0;
    }
    return 1;
}

static void add_conv_to_trt(ln_op *trt_op, const ln_op *op)
{
    ln_param_entry *pe;
    ln_tensor_entry *te;
    char *tensor_name;
    ln_op_arg *trt_arg = trt_op->op_arg;
    ln_op_arg *op_arg = op->op_arg;
    char *param_op_arg_name;
    char *param_arg_name;

    param_op_arg_name = create_arg_name_in_params(trt_arg->params, "op");
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  param_op_arg_name, "conv");

    add_src(trt_arg, op_arg, param_op_arg_name, "src", "src");
    add_weight(trt_arg, op_arg, param_op_arg_name, "weight", "weight");
    add_weight(trt_arg, op_arg, param_op_arg_name, "bias", "bias");
    add_dst(trt_arg, op_arg, param_op_arg_name, "dst", "dst");

    tensor_name = ln_tensor_list_find_name(op_arg->tensors_in, "src");
    te = ln_tensor_table_find(op_arg->tensor_table, tensor_name);
    check_and_add_batch_size(trt_op, te->tensor->dims[0], op_arg->name);

    pe = ln_param_list_find(op_arg->params, "group");
    param_arg_name = ln_strcat_delim_alloc(param_op_arg_name, "group", '_');
    trt_arg->params = ln_param_list_append_number(trt_arg->params,
                                                  param_arg_name, pe->value_double);
    ln_free(param_arg_name);

    tensor_name = ln_tensor_list_find_name(op_arg->tensors_out, "dst");
    te = ln_tensor_table_find(op_arg->tensor_table, tensor_name);
    param_arg_name = ln_strcat_delim_alloc(param_op_arg_name, "output_c", '_');
    trt_arg->params = ln_param_list_append_number(trt_arg->params,
                                                  param_arg_name, te->tensor->dims[1]);
    ln_free(param_arg_name);

    pe = ln_param_list_find(op_arg->params, "size");
    param_arg_name = ln_strcat_delim_alloc(param_op_arg_name, "size", '_');
    trt_arg->params = ln_param_list_append_array_number(trt_arg->params,
                                                        param_arg_name, 2,
                                                        pe->value_array_double);
    ln_free(param_arg_name);

    pe = ln_param_list_find(op_arg->params, "stride");
    param_arg_name = ln_strcat_delim_alloc(param_op_arg_name, "stride", '_');
    trt_arg->params = ln_param_list_append_array_number(trt_arg->params,
                                                        param_arg_name, 2,
                                                        pe->value_array_double);
    ln_free(param_arg_name);

    pe = ln_param_list_find(op_arg->params, "padding");
    param_arg_name = ln_strcat_delim_alloc(param_op_arg_name, "padding", '_');
    trt_arg->params = ln_param_list_append_array_number(trt_arg->params,
                                                        param_arg_name, 2,
                                                        (double[]){pe->value_array_double[0],
                                                                pe->value_array_double[2]});
    ln_free(param_arg_name);

    pe = ln_param_list_find(op_arg->params, "dilation");
    param_arg_name = ln_strcat_delim_alloc(param_op_arg_name, "dilation", '_');
    trt_arg->params = ln_param_list_append_array_number(trt_arg->params,
                                                        param_arg_name, 2,
                                                        pe->value_array_double);
    ln_free(param_arg_name);

    ln_free(param_op_arg_name);
}

static void add_activation_to_trt(ln_op *trt_op, const ln_op *op)
{
    ln_op_arg *trt_arg = trt_op->op_arg;
    ln_op_arg *op_arg = op->op_arg;
    ln_tensor_entry *te;
    char *param_op_arg_name;
    const char *atype;
    char *param_arg_name;

    param_op_arg_name = create_arg_name_in_params(trt_arg->params, "op");
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  param_op_arg_name, "activation");

    add_src(trt_arg, op_arg, param_op_arg_name, "src", "src");
    add_dst(trt_arg, op_arg, param_op_arg_name, "dst", "dst");
    te = ln_op_find_tensor_entry(op, "src");
    check_and_add_batch_size(trt_op, te->tensor->dims[0], op_arg->name);

    if (ln_streq(op_arg->optype, "relu"))
        atype = "kRELU";
    else if (ln_streq(op_arg->optype, "sigmoid"))
        atype = "kSIGMOID";
    else if (ln_streq(op_arg->optype, "tanh"))
        atype = "kTANH";
    else
        assert(0 && "unsupported activation type");

    param_arg_name = ln_strcat_delim_alloc(param_op_arg_name, "activation_type", '_');
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  param_arg_name,
                                                  atype);
    ln_free(param_arg_name);
    ln_free(param_op_arg_name);
}

static int check_pooling(const ln_op *op)
{
    ln_param_entry *pe;

    pe = ln_param_list_find(op->op_arg->params, "padding");
    if (pe->value_array_int[0] != pe->value_array_int[2]
        || pe->value_array_int[1] != pe->value_array_int[3]) {
        ln_msg_emit(LN_WARNING,
                      "cannot convert '%s' with asymmetrical padding to TensorRT pooling op",
                      op->op_arg->name);
        return 0;
    }
    return 1;
}

static void add_pooling_to_trt(ln_op *trt_op, const ln_op *op)
{
    ln_op_arg *trt_arg = trt_op->op_arg;
    ln_op_arg *op_arg = op->op_arg;
    char *param_op_arg_name;
    char *tensor_name;
    ln_tensor_entry *te;
    const char *pool_type;
    char *param_arg_name;
    ln_param_entry *pe;

    param_op_arg_name = create_arg_name_in_params(trt_arg->params, "op");
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  param_op_arg_name, "pooling");

    add_src(trt_arg, op_arg, param_op_arg_name, "src", "src");
    add_dst(trt_arg, op_arg, param_op_arg_name, "dst", "dst");

    tensor_name = ln_tensor_list_find_name(op_arg->tensors_in, "src");
    te = ln_tensor_table_find(op_arg->tensor_table, tensor_name);
    check_and_add_batch_size(trt_op, te->tensor->dims[0], op_arg->name);

    if (ln_streq(op_arg->optype, "maxpool2d"))
        pool_type = "kMAX";
    else if (ln_streq(op_arg->optype, "averagepool2d"))
        pool_type = "kAVERAGE";
    else
        assert(0 && "unsupported pooling type");

    param_arg_name = ln_strcat_delim_alloc(param_op_arg_name, "pooling_type", '_');
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  param_arg_name,
                                                  pool_type);
    ln_free(param_arg_name);

    pe = ln_param_list_find(op_arg->params, "size");
    param_arg_name = ln_strcat_delim_alloc(param_op_arg_name, "size", '_');
    trt_arg->params = ln_param_list_append_array_number(trt_arg->params,
                                                        param_arg_name, 2,
                                                        pe->value_array_double);
    ln_free(param_arg_name);

    pe = ln_param_list_find(op_arg->params, "stride");
    param_arg_name = ln_strcat_delim_alloc(param_op_arg_name, "stride", '_');
    trt_arg->params = ln_param_list_append_array_number(trt_arg->params,
                                                        param_arg_name, 2,
                                                        pe->value_array_double);
    ln_free(param_arg_name);

    pe = ln_param_list_find(op_arg->params, "padding");
    param_arg_name = ln_strcat_delim_alloc(param_op_arg_name, "padding", '_');
    trt_arg->params = ln_param_list_append_array_number(trt_arg->params,
                                                        param_arg_name, 2,
                                                        (double[]){pe->value_array_double[0],
                                                                pe->value_array_double[2]});
    ln_free(param_arg_name);

    ln_free(param_op_arg_name);
}

static int check_softmax(const ln_op *op)
{
    ln_param_entry *pe;
    ln_tensor_entry *te;

    pe = ln_param_list_find(op->op_arg->params, "axis");
    te = ln_tensor_table_find(op->op_arg->tensor_table,
                              ln_tensor_list_find_name(op->op_arg->tensors_in,
                                                       "src"));
    assert(te);

    if (pe->value_int == 0) {
        ln_msg_emit(LN_WARNING,
                      "cannot convert '%s' with 'axis' = 0 to TensorRT softmax op",
                      op->op_arg->name);
        return 0;
    }
    if (strverscmp(ln_tensorrt_version_str(), "4.0.0") < 0) {
        if (te->tensor->ndim < 4 && pe->value_int != 1) {
            ln_msg_emit(LN_WARNING,
                          "cannot convert '%s' with 'src''s number of dimensions < 4 and 'axis' != 1 to TensorRT softmax op in TensorRT version < 4.0.0",
                          op->op_arg->name);
            return 0;
        }
        if (te->tensor->ndim >= 4 && pe->value_int != te->tensor->ndim-3) {
            ln_msg_emit(LN_WARNING,
                          "cannot convert '%s' with 'src''s number of dimensions >= 4 and 'axis' != 'src''s number of dimensions minus 3 to TensorRT softmax op in TensorRT version < 4.0.0",
                          op->op_arg->name);
            return 0;
        }
    }
    return 1;
}

static void add_softmax_to_trt(ln_op *trt_op, const ln_op *op)
{
    ln_op_arg *trt_arg = trt_op->op_arg;
    ln_op_arg *op_arg = op->op_arg;
    char *param_op_arg_name;
    char *tensor_name;
    ln_tensor_entry *te;
    ln_param_entry *pe;
    int axes = 0;
    char *param_arg_name;

    param_op_arg_name = create_arg_name_in_params(trt_arg->params, "op");
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  param_op_arg_name, "softmax");

    add_src(trt_arg, op_arg, param_op_arg_name, "src", "src");
    add_dst(trt_arg, op_arg, param_op_arg_name, "dst", "dst");

    tensor_name = ln_tensor_list_find_name(op_arg->tensors_in, "src");
    te = ln_tensor_table_find(op_arg->tensor_table, tensor_name);
    check_and_add_batch_size(trt_op, te->tensor->dims[0], op_arg->name);

    pe = ln_param_list_find(op->op_arg->params, "axis");

    if (strverscmp(ln_tensorrt_version_str(), "4.0.0") >= 0) {
        /* axes |= 1 << (pe->value_int - 1); */
        axes |= 1 << (pe->value_int); /* TODO: FIXME: - 1 or not?? */
        param_arg_name = ln_strcat_delim_alloc(param_op_arg_name, "axes", '_');
        trt_arg->params = ln_param_list_append_number(trt_arg->params,
                                                      param_arg_name, axes);
        ln_free(param_arg_name);
    }

    ln_free(param_op_arg_name);
}

static int check_concat(const ln_op *op)
{
    ln_param_entry *pe;
    ln_tensor_entry *te;

    pe = ln_param_list_find(op->op_arg->params, "axis");
    te = ln_tensor_table_find(op->op_arg->tensor_table,
                              ln_tensor_list_find_name(op->op_arg->tensors_in,
                                                       "src1"));
    assert(te);

    if (pe->value_int == 0) {
        ln_msg_emit(LN_WARNING,
                      "cannot convert '%s' with 'axis' = 0 to TensorRT concat op",
                      op->op_arg->name);
        return 0;
    }
    if (strverscmp(ln_tensorrt_version_str(), "4.0.0") < 0) {
        if (te->tensor->ndim < 4 && pe->value_int != 1) {
            ln_msg_emit(LN_WARNING,
                          "cannot convert '%s' with 'src1''s number of dimensions < 4 and 'axis' != 1 to TensorRT concat op in TensorRT version < 4.0.0",
                          op->op_arg->name);
            return 0;
        }
        if (te->tensor->ndim >= 4 && pe->value_int != te->tensor->ndim-3) {
            ln_msg_emit(LN_WARNING,
                          "cannot convert '%s' with 'src1''s number of dimensions >= 4 and 'axis' != 'src1''s number of dimensions minus 3 to TensorRT concat op in TensorRT version < 4.0.0",
                          op->op_arg->name);
            return 0;
        }
    }
    return 1;
}

static void add_concat_to_trt(ln_op *trt_op, const ln_op *op)
{
    ln_op_arg *trt_arg = trt_op->op_arg;
    ln_op_arg *op_arg = op->op_arg;
    char *param_op_arg_name;
    char *tensor_name;
    ln_tensor_entry *te;
    ln_param_entry *pe;
    int axis;
    char *param_arg_name;

    param_op_arg_name = create_arg_name_in_params(trt_arg->params, "op");
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  param_op_arg_name, "concat");

    add_src(trt_arg, op_arg, param_op_arg_name, "src1", "src1");
    add_src(trt_arg, op_arg, param_op_arg_name, "src2", "src2");
    add_dst(trt_arg, op_arg, param_op_arg_name, "dst", "dst");

    tensor_name = ln_tensor_list_find_name(op_arg->tensors_in, "src1");
    te = ln_tensor_table_find(op_arg->tensor_table, tensor_name);
    check_and_add_batch_size(trt_op, te->tensor->dims[0], op_arg->name);
    tensor_name = ln_tensor_list_find_name(op_arg->tensors_in, "src2");
    te = ln_tensor_table_find(op_arg->tensor_table, tensor_name);
    check_and_add_batch_size(trt_op, te->tensor->dims[0], op_arg->name);

    pe = ln_param_list_find(op->op_arg->params, "axis");

    if (strverscmp(ln_tensorrt_version_str(), "4.0.0") >= 0) {
        /* axis = pe->value_int - 1; */
        axis = pe->value_int;   /* TODO: FIXME: - 1 or not?? */
        param_arg_name = ln_strcat_delim_alloc(param_op_arg_name, "axis", '_');
        trt_arg->params = ln_param_list_append_number(trt_arg->params,
                                                      param_arg_name, axis);
        ln_free(param_arg_name);
    }

    ln_free(param_op_arg_name);
}

static void add_batchnorm_weight(ln_op_arg *trt_arg, ln_op_arg *bn2scale_arg,
                                 char *opname, char *origin_tensor_arg,
                                 char *param_arg)
{
    char *tensor_name;
    char *param_arg_name;
    char *tensor_arg_name;

    /* the difference is that here we use bn2scale_arg->tensors_out */
    tensor_name = ln_tensor_list_find_name(bn2scale_arg->tensors_out,
                                           origin_tensor_arg);
    param_arg_name = ln_strcat_delim_alloc(opname, param_arg, '_');
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  param_arg_name, tensor_name);
    ln_free(param_arg_name);

    if (!exists_in_tensors(trt_arg->tensors_in, tensor_name)) {
        tensor_arg_name = create_arg_name_in_tensors(trt_arg->tensors_in,
                                                     "weight");
        trt_arg->tensors_in = ln_tensor_list_append(trt_arg->tensors_in,
                                                    tensor_arg_name,
                                                    tensor_name);
        ln_free(tensor_arg_name);
    }
}

static ln_op *op_create_by_optype(const ln_op *op, const char *new_optype)
{
    ln_op *new_op_proto;
    ln_op *new_op;

    new_op_proto = ln_hash_find(LN_ARCH.op_proto_table, new_optype);
    new_op = ln_op_create_from_proto(new_op_proto, op->op_arg->name,
                                     ln_tensor_list_copy(op->op_arg->tensors_in),
                                     ln_tensor_list_copy(op->op_arg->tensors_out),
                                     ln_param_list_copy(op->op_arg->params),
                                     op->op_arg->tensor_table);
    return new_op;
}

static ln_list *simple_replace(const ln_op *op, const char *new_optype)
{
    ln_op *new_op;

    new_op = op_create_by_optype(op, new_optype);
    return ln_list_append(NULL, new_op);
}

static ln_list *ep_create(const ln_op *op, const ln_dfg *dfg, int *match)
{
    ln_op *next_op;
    ln_tensor_list_entry *tle;
    ln_tensor_list_entry *tle_next;
    char *new_optype;

    *match = 1;

    tle = ln_tensor_list_find_by_arg_name(op->op_arg->tensors_out, "dst");
    next_op = ln_dfg_next(dfg, op, tle->name);
    assert(next_op);
    tle_next = ln_tensor_list_find_by_name(next_op->op_arg->tensors_in,
                                           tle->name);
    assert(tle_next);

    /* TODO: FIXME: this may not be right in the future */
    if (ln_streqn(tle_next->arg_name, "src", 3) &&
        (ln_streq(next_op->op_arg->optype, "conv2d") ||
         ln_streq(next_op->op_arg->optype, "relu") ||
         ln_streq(next_op->op_arg->optype, "maxpool2d") ||
         ln_streq(next_op->op_arg->optype, "softmax") ||
         ln_streq(next_op->op_arg->optype, "concat") ||
         ln_streq(next_op->op_arg->optype, "tensorrt")))
        new_optype = "create_cuda";
    else
        new_optype = "create_cpu";

    return simple_replace(op, new_optype);
}

static ln_list *ep_conv2d(const ln_op *op, const ln_dfg *dfg, int *match)
{
    ln_op *trt_op;

    *match = 1;

    if (!check_conv(op))
        return simple_replace(op, "conv2d_cuda");

    trt_op = ln_op_create_with_opname(&ln_opimpl_tensorrt,
                                     op->op_arg->tensor_table);
    add_conv_to_trt(trt_op, op);

    return ln_list_append(NULL, trt_op);
}

static ln_list *ep_relu(const ln_op *op, const ln_dfg *dfg, int *match)
{
    ln_op *trt_op;

    *match = 1;

    trt_op = ln_op_create_with_opname(&ln_opimpl_tensorrt,
                                     op->op_arg->tensor_table);
    add_activation_to_trt(trt_op, op);

    return ln_list_append(NULL, trt_op);
}

static ln_list *ep_maxpool2d(const ln_op *op, const ln_dfg *dfg, int *match)
{
    ln_op *trt_op;

    *match = 1;

    if (!check_pooling(op))
        return simple_replace(op, "maxpool2d_cuda");

    trt_op = ln_op_create_with_opname(&ln_opimpl_tensorrt,
                                     op->op_arg->tensor_table);
    add_pooling_to_trt(trt_op, op);

    return ln_list_append(NULL, trt_op);
}

static ln_list *ep_softmax(const ln_op *op, const ln_dfg *dfg, int *match)
{
    ln_op *trt_op;

    *match = 1;

    if (!check_softmax(op))
        return simple_replace(op, "softmax_cuda");

    trt_op = ln_op_create_with_opname(&ln_opimpl_tensorrt,
                                     op->op_arg->tensor_table);
    add_softmax_to_trt(trt_op, op);

    return ln_list_append(NULL, trt_op);
}

static ln_list *ep_concat(const ln_op *op, const ln_dfg *dfg, int *match)
{
    ln_op *trt_op;

    *match = 1;

    if (!check_concat(op))
        return simple_replace(op, "concat_cuda");

    trt_op = ln_op_create_with_opname(&ln_opimpl_tensorrt,
                                     op->op_arg->tensor_table);
    add_concat_to_trt(trt_op, op);

    return ln_list_append(NULL, trt_op);
}

static ln_list *ep_batchnorm(const ln_op *op, const ln_dfg *dfg, int *match)
{
    ln_list *new_ops = NULL;
    ln_op *bn2scale_op, *trt_op;
    ln_op *op_proto;
    ln_list *params = NULL;
    ln_param_entry *pe;
    ln_tensor_entry *te;
    ln_tensor_list_entry *tle;
    char *op_arg_name;
    char *arg_name;

    *match = 1;

    op_proto = ln_hash_find(LN_ARCH.op_proto_table, "bn2scale_wts_cpu");
    assert(op_proto);
    bn2scale_op = ln_op_create_with_names(op_proto, op->op_arg->tensor_table);
    tle = ln_op_find_tensor_list_entry(bn2scale_op, "src_mean");
    ln_free(tle->name);
    te = ln_op_find_tensor_entry(op, "mean");
    tle->name = ln_strdup(te->name);
    tle = ln_op_find_tensor_list_entry(bn2scale_op, "src_var");
    ln_free(tle->name);
    te = ln_op_find_tensor_entry(op, "var");
    tle->name = ln_strdup(te->name);
    tle = ln_op_find_tensor_list_entry(bn2scale_op, "src_scale");
    ln_free(tle->name);
    te = ln_op_find_tensor_entry(op, "scale");
    tle->name = ln_strdup(te->name);
    tle = ln_op_find_tensor_list_entry(bn2scale_op, "src_offset");
    ln_free(tle->name);
    te = ln_op_find_tensor_entry(op, "offset");
    tle->name = ln_strdup(te->name);
    pe = ln_param_list_find(op->op_arg->params, "epsilon");
    params = ln_param_list_append_float(params, "epsilon", pe->value_float);
    bn2scale_op->op_arg->params = params;
    new_ops = ln_list_append(new_ops, bn2scale_op);

    op_proto = &ln_opimpl_tensorrt;
    trt_op = ln_op_create_with_opname(op_proto, op->op_arg->tensor_table);
    te = ln_op_find_tensor_entry(op, "src");
    assert(te);
    check_and_add_batch_size(trt_op, te->tensor->dims[0], op->op_arg->name);
    op_arg_name = create_arg_name_in_params(trt_op->op_arg->params, "op");
    trt_op->op_arg->params = ln_param_list_append_string(trt_op->op_arg->params,
                                                         op_arg_name, "scale");
    add_src(trt_op->op_arg, op->op_arg, op_arg_name, "src", "src");
    add_dst(trt_op->op_arg, op->op_arg, op_arg_name, "dst", "dst");
    add_batchnorm_weight(trt_op->op_arg, bn2scale_op->op_arg, op_arg_name,
                         "dst_shift", "shift");
    add_batchnorm_weight(trt_op->op_arg, bn2scale_op->op_arg, op_arg_name,
                         "dst_scale", "scale");
    add_batchnorm_weight(trt_op->op_arg, bn2scale_op->op_arg, op_arg_name,
                         "dst_power", "power");
    arg_name = ln_strcat_delim_alloc(op_arg_name, "scale_mode", '_');
    trt_op->op_arg->params = ln_param_list_append_string(trt_op->op_arg->params,
                                                         arg_name, "kCHANNEL");
    ln_free(arg_name);
    ln_free(op_arg_name);

    new_ops = ln_list_append(new_ops, trt_op);
    return new_ops;
}

static ln_list *ep_elew(const ln_op *op, const ln_dfg *dfg, int *match)
{
    *match = 1;
    return simple_replace(op, "elew_cuda");
}

static ln_list *ep_maxreduce(const ln_op *op, const ln_dfg *dfg, int *match)
{
    *match = 1;
    return simple_replace(op, "maxreduce_cuda");
}

static ln_list *ep_maxreduce_arg(const ln_op *op, const ln_dfg *dfg, int *match)
{
    *match = 1;
    return simple_replace(op, "maxreduce_arg_cuda");
}

static ln_list *ep_slice(const ln_op *op, const ln_dfg *dfg, int *match)
{
    *match = 1;
    return simple_replace(op, "slice_cuda");
}

static ln_list *ep_transpose(const ln_op *op, const ln_dfg *dfg, int *match)
{
    *match = 1;
    return simple_replace(op, "transpose_cuda");
}

static ln_list *ep_upsample(const ln_op *op, const ln_dfg *dfg, int *match)
{
    *match = 1;
    return simple_replace(op, "upsample_cuda");
}

static ln_list *ep_zeros(const ln_op *op, const ln_dfg *dfg, int *match)
{
    *match = 1;
    return simple_replace(op, "zeros_cuda");
}

static ln_list *ep_reshape(const ln_op *op, const ln_dfg *dfg, int *match)
{
    *match = 1;
    return simple_replace(op, "reshape_cuda");
}

static ln_list *ep_print(const ln_op *op, const ln_dfg *dfg, int *match)
{
    ln_tensor_list_entry *tle;
    ln_op *prev_op;
    const char *optype;

    *match = 1;
    tle = ln_tensor_list_find_by_arg_name(op->op_arg->tensors_in, "src");
    prev_op = ln_dfg_prev(dfg, op, tle->name);
    assert(prev_op);
    if (ln_streq(prev_op->op_arg->arch, "cpu"))
        optype = "print_cpu";
    else if (ln_streq(prev_op->op_arg->arch, "cuda"))
        optype = "print_cuda";
    else
        assert(0 && "print's prev op is either of cpu or cuda");

    return simple_replace(op, optype);
}

static ln_list *ep_tensorrt(const ln_op *op, const ln_dfg *dfg, int *match)
{
    /* don't touch it in expanding */
    *match = 0;
    return NULL;
}

static ln_list *ep_func_tensorrt(const ln_op *op, const ln_dfg *dfg, int *match)
{
    ln_list *new_ops;
    ln_expander_func ep_func;
    void *value;

    if (!ln_hash_find_extended(ep_funcs_hash, op->op_arg->optype, NULL, &value))
        ln_msg_inter_error(0, "unsupported optype \"%s\" for TensorRT optimization",
                           op->op_arg->optype);

    ep_func = value;
    new_ops = ep_func(op, dfg, match);

    return new_ops;
}

static void add_trt_src(ln_op_arg *trt_arg, ln_op_arg *arg, char *tensor_name)
{
    char *tensor_arg_name;

    if (!exists_in_tensors(trt_arg->tensors_out, tensor_name) &&
        !exists_in_tensors(trt_arg->tensors_in, tensor_name)) {
        tensor_arg_name = create_arg_name_in_tensors(trt_arg->tensors_in,
                                                     "src");
        trt_arg->tensors_in = ln_tensor_list_append(trt_arg->tensors_in,
                                                    tensor_arg_name,
                                                    tensor_name);
        ln_free(tensor_arg_name);
    }
}

static void add_trt_weight(ln_op_arg *trt_arg, ln_op_arg *arg, char *tensor_name)
{
    char *tensor_arg_name;

    if (!exists_in_tensors(trt_arg->tensors_in, tensor_name)) {
        tensor_arg_name = create_arg_name_in_tensors(trt_arg->tensors_in,
                                                     "weight");
        trt_arg->tensors_in = ln_tensor_list_append(trt_arg->tensors_in,
                                                    tensor_arg_name,
                                                    tensor_name);
        ln_free(tensor_arg_name);
    }
}

static void add_trt_dst(ln_op_arg *trt_arg, ln_op_arg *arg, char *tensor_name)
{
    char *param_arg_name;
    char *tensor_arg_name;
    ln_tensor_entry *te;

    if (exists_in_tensors(trt_arg->tensors_out, tensor_name))
        return;

    tensor_arg_name = create_arg_name_in_tensors(trt_arg->tensors_out, "dst");
    trt_arg->tensors_out = ln_tensor_list_append(trt_arg->tensors_out,
                                                 tensor_arg_name,
                                                 tensor_name);

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

static int digits_len(int n)
{
    int count = 0;

    while (n) {
        n /= 10;
        count++;
    }
    return n == 0 ? 1 : count;
}

static char *new_arg_name(const char *old_name, int base_idx)
{
    char *new_name;
    char *next_token;
    int old_idx, new_idx;
    size_t new_name_len;

    old_idx = atoi(&old_name[2]); /* 2 for length of "op" */
    new_idx = old_idx + base_idx;
    new_name_len = strlen(old_name) - digits_len(old_idx) + digits_len(new_idx);
    new_name = ln_alloc(sizeof(char) * (new_name_len + 1));

    if ((next_token = ln_next_token(old_name, '_')))
        snprintf(new_name, new_name_len + 1, "op%d_%s", new_idx, next_token);
    else
        snprintf(new_name, new_name_len + 1, "op%d", new_idx);

    return new_name;
}

static int pe_cmp(const void *p1, const void *p2)
{
    const ln_param_entry *pe1 = p1;
    const ln_param_entry *pe2 = p2;

    return strcmp(pe1->arg_name, pe2->arg_name);
}

static void pe_free(void *p)
{
    ln_param_entry_free(p);
}

static int have_successor_except(const ln_op *trt_op, const char *tname,
                                 const ln_op *op, const ln_dfg *dfg)
{
    ln_list *suc_ens;
    ln_graph_edge_node *en;
    ln_op *suc_op;
    int ret = 0;

    suc_ens = ln_dfg_nexts(dfg, trt_op, tname);
    if (!suc_ens) {
        ret = 0;
        goto end;
    }
    LN_LIST_FOREACH(en, suc_ens) {
        suc_op = en->node->data;
        if (!ln_streq(suc_op->op_arg->name, op->op_arg->name)) {
            ret = 1;
            goto end;
        }
    }
    ret = 0;

end:
    ln_list_free(suc_ens);
    return ret;
}

/* remove dsts that don't have a successor except `op` */
/* NOTE: Unable to remove the lastest-added op's dst, which usually always have
   a successor. But if not, this will waste a little GPU memory for that dst. */
static void remove_extra_dst(ln_op *trt_op, const ln_op *op, const ln_dfg *dfg)
{
    ln_list **lp;
    ln_list *tmp;
    ln_tensor_list_entry *tle;
    ln_param_entry pe;
    ln_op_arg *trt_op_arg = trt_op->op_arg;

    for (lp = &trt_op->op_arg->tensors_out; *lp;) {
        tle = (*lp)->data;
        if (have_successor_except(trt_op, tle->name, op, dfg)) {
            lp = &(*lp)->next;
            continue;
        }

        pe.arg_name = ln_strcat_delim_alloc(tle->arg_name, "shape", '_');
        trt_op_arg->params = ln_list_remove_custom_deep(trt_op_arg->params,
                                                        &pe, pe_cmp, pe_free);
        ln_free(pe.arg_name);

        pe.arg_name = ln_strcat_delim_alloc(tle->arg_name, "dtype", '_');
        trt_op_arg->params = ln_list_remove_custom_deep(trt_op_arg->params,
                                                        &pe, pe_cmp, pe_free);
        ln_free(pe.arg_name);

        tmp = *lp;
        *lp = tmp->next;
        ln_tensor_list_entry_free(tle);
        ln_free(tmp);
    }
}

static void add_trt_to_trt(ln_op *trt_op, const ln_op *op, const ln_dfg *dfg)
{
    ln_op_arg *trt_arg = trt_op->op_arg;
    ln_op_arg *op_arg = op->op_arg;
    ln_param_entry *pe;
    ln_param_entry *new_pe;
    ln_tensor_list_entry *tle;
    char *param_op_arg_name;
    int op_batch_size;
    int base_idx;

    pe = ln_param_list_find(op_arg->params, "batch_size");
    op_batch_size = pe->value_int;
    pe = ln_param_list_find(trt_arg->params, "batch_size");
    if (pe && op_batch_size != pe->value_int) {
        ln_msg_emit(LN_ERROR,
                      "batch size doesn't match among ops when converting to TensorRT: original batch_size = %d, '%s''s batch_size = %d",
                      pe->value_int, op->op_arg->name, op_batch_size);
    } else if (!pe) {
        trt_arg->params = ln_param_list_append_int(trt_arg->params,
                                                   "batch_size", op_batch_size);
    }

    LN_LIST_FOREACH(tle, op_arg->tensors_in) {
        if (ln_streqn(tle->arg_name, "src", 3))
            add_trt_src(trt_arg, op_arg, tle->name);
        else
            add_trt_weight(trt_arg, op_arg, tle->name);
    }

    remove_extra_dst(trt_op, op, dfg);

    LN_LIST_FOREACH(tle, op_arg->tensors_out) {
        add_trt_dst(trt_arg, op_arg, tle->name);
    }

    param_op_arg_name = create_arg_name_in_params(trt_arg->params, "op");
    base_idx = atoi(&param_op_arg_name[2]);
    ln_free(param_op_arg_name);

    LN_LIST_FOREACH(pe, op_arg->params) {
        if (!ln_streqn(pe->arg_name, "op", 2))
            continue;
        new_pe = ln_param_entry_copy(pe);
        ln_free(new_pe->arg_name);
        new_pe->arg_name = new_arg_name(pe->arg_name, base_idx);
        trt_arg->params = ln_list_append(trt_arg->params, new_pe);
    }
}

static int is_win_match(const ln_list *win_ops, size_t win_size)
{
    ln_op *op1;
    ln_op *op2;
    ln_list *l;
    size_t i;

    /* two tensorrt ops in a row is a match */
    for (i = 0, l = (ln_list *)win_ops; i + 1 < win_size && l && l->next;
         l = l->next, i++) {
        op1 = l->data;
        op2 = l->next->data;
        if (ln_streq(op1->op_arg->optype, "tensorrt") &&
            ln_streq(op2->op_arg->optype, "tensorrt"))
            return 1;
    }
    return 0;
}

static ln_list *cb_func_tensorrt(const ln_list *win_ops, size_t win_size,
                                 const ln_dfg *dfg, int *match)
{
    ln_list *new_ops = NULL;
    ln_list *l;
    ln_op *op;
    ln_op *trt_op;
    ln_op *no_trt_op;
    size_t i;

    *match = 0;
    if (is_win_match(win_ops, win_size))
        *match = 1;
    else
        return NULL;

    for (i = 0, l = (ln_list *)win_ops; i < win_size && l;) {
        op = l->data;
        if (!ln_streq(op->op_arg->optype, "tensorrt")) {
            i++, l = l->next;
            no_trt_op = ln_op_copy(op);
            new_ops = ln_list_append(new_ops, no_trt_op);
            continue;
        }
        trt_op = ln_op_copy(op);
        for (i++, l = l->next; i < win_size && l; i++, l = l->next) {
            op = l->data;
            if (!ln_streq(op->op_arg->optype, "tensorrt"))
                break;
            add_trt_to_trt(trt_op, op, dfg);
        }
        new_ops = ln_list_append(new_ops, trt_op);
    }

    return new_ops;
}

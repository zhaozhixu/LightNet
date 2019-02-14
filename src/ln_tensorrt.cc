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

#include <NvInfer.h>
#include <iostream>
#include <map>

#include "ln_op.h"
#include "ln_tensorrt.h"

#if NV_TENSORRT_MAJOR < 2
#error TensorRT version below 2.x.x is not supported.
#endif

using namespace nvinfer1;

static char TENSORRT_VERSION_STR[20] = {0};

// TODO: add async support
struct ln_tensorrt_bundle {
    ICudaEngine        *engine;
    IExecutionContext  *context;
    void              **bindings;
    int                 batch_size;
};

const char *ln_tensorrt_version_str(void)
{
    if (TENSORRT_VERSION_STR[0] == 0)
        snprintf(TENSORRT_VERSION_STR, 20, "%d.%d.%d",
                 NV_TENSORRT_MAJOR, NV_TENSORRT_MINOR, NV_TENSORRT_PATCH);
    return TENSORRT_VERSION_STR;
}

static int tl_dtype_to_ioTensor_DataType(tl_dtype dtype)
{
    // TODO: add fp16 support
    switch (dtype) {
    case TL_FLOAT:
        return (int)DataType::kFLOAT;
#if NV_TENSORRT_MAJOR >= 4
    case TL_INT32:
        return (int)DataType::kINT32;
#endif
    default:
        return -1;
    }
}

static int tl_dtype_to_weight_DataType(tl_dtype dtype)
{
    // TODO: add fp16 support
    switch (dtype) {
    case TL_FLOAT:
        return (int)DataType::kFLOAT;
#if NV_TENSORRT_MAJOR >= 4
    case TL_INT32:
        return (int)DataType::kINT32;
#endif
    case TL_INT8:
        return (int)DataType::kINT8;
    default:
        return -1;
    }
}

static int str_to_activation_type(const char *str)
{
    if (ln_streq(str, "kRELU"))
        return (int)ActivationType::kRELU;
    if (ln_streq(str, "kSIGMOID"))
        return (int)ActivationType::kSIGMOID;
    if (ln_streq(str, "kTANH"))
        return (int)ActivationType::kTANH;
    return -1;
}

static int str_to_pooling_type(const char *str)
{
    if (ln_streq(str, "kMAX"))
        return (int)PoolingType::kMAX;
    if (ln_streq(str, "kAVERAGE"))
        return (int)PoolingType::kAVERAGE;
    if (ln_streq(str, "kMAX_AVERAGE_BLEND"))
        return (int)PoolingType::kMAX_AVERAGE_BLEND;
    return -1;
}

static int str_to_elew_type(const char *str)
{
    if (ln_streq(str, "kSUM"))
        return (int)ElementWiseOperation::kSUM;
    if (ln_streq(str, "kPROD"))
        return (int)ElementWiseOperation::kPROD;
    if (ln_streq(str, "kMAX"))
        return (int)ElementWiseOperation::kMAX;
    if (ln_streq(str, "kMIN"))
        return (int)ElementWiseOperation::kMIN;
    if (ln_streq(str, "kSUB"))
        return (int)ElementWiseOperation::kSUB;
    if (ln_streq(str, "kDIV"))
        return (int)ElementWiseOperation::kDIV;
    if (ln_streq(str, "kPOW"))
        return (int)ElementWiseOperation::kPOW;
    return -1;
}

static int str_to_scale_mode(const char *str)
{
    if (ln_streq(str, "kUNIFORM"))
        return (int)ScaleMode::kUNIFORM;
    if (ln_streq(str, "kCHANNEL"))
        return (int)ScaleMode::kCHANNEL;
    if (ln_streq(str, "kELEMENTWISE"))
        return (int)ScaleMode::kELEMENTWISE;
    return -1;
}

static void check_param(const char *name1, const char *name2,
                        ln_param_type ptype, int plen, ln_op_arg *op_arg)
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
static void check_conv(char *opname, ln_op_arg *op_arg)
{
    check_param(opname, "src", LN_PARAM_STRING, 0, op_arg);
    check_param(opname, "weight", LN_PARAM_STRING, 0, op_arg);
    check_param(opname, "bias", LN_PARAM_STRING, 0, op_arg);
    check_param(opname, "dst", LN_PARAM_STRING, 0, op_arg);
    check_param(opname, "group", LN_PARAM_NUMBER, 0, op_arg);
    check_param(opname, "output_c", LN_PARAM_NUMBER, 0, op_arg);
    check_param(opname, "size", LN_PARAM_ARRAY_NUMBER, 2, op_arg);
    check_param(opname, "stride", LN_PARAM_ARRAY_NUMBER, 2, op_arg);
    check_param(opname, "padding", LN_PARAM_ARRAY_NUMBER, 2, op_arg);
    check_param(opname, "dilation", LN_PARAM_ARRAY_NUMBER, 2, op_arg);
}

static void check_activation(char *opname, ln_op_arg *op_arg)
{
    check_param(opname, "src", LN_PARAM_STRING, 0, op_arg);
    check_param(opname, "dst", LN_PARAM_STRING, 0, op_arg);
    check_param(opname, "activation_type", LN_PARAM_STRING, 0, op_arg);

    ln_param_entry *pe;
    pe = ln_param_list_find2(op_arg->params, opname, "activation_type");
    ln_opck_satisfy_msg(str_to_activation_type(pe->value_string) != -1,
                        "unsupported activation type");
}

static void check_pooling(char *opname, ln_op_arg *op_arg)
{
    check_param(opname, "src", LN_PARAM_STRING, 0, op_arg);
    check_param(opname, "dst", LN_PARAM_STRING, 0, op_arg);
    check_param(opname, "size", LN_PARAM_ARRAY_NUMBER, 2, op_arg);
    check_param(opname, "stride", LN_PARAM_ARRAY_NUMBER, 2, op_arg);
    check_param(opname, "padding", LN_PARAM_ARRAY_NUMBER, 2, op_arg);
    check_param(opname, "pooling_type", LN_PARAM_STRING, 0, op_arg);

    ln_param_entry *pe;
    pe = ln_param_list_find2(op_arg->params, opname, "pooling_type");
    ln_opck_satisfy_msg(str_to_pooling_type(pe->value_string) != -1,
                        "unsupported pooling type");
}

static void check_softmax(char *opname, ln_op_arg *op_arg)
{
    check_param(opname, "src", LN_PARAM_STRING, 0, op_arg);
    check_param(opname, "dst", LN_PARAM_STRING, 0, op_arg);
}

static void check_elew(char *opname, ln_op_arg *op_arg)
{
    check_param(opname, "src1", LN_PARAM_STRING, 0, op_arg);
    check_param(opname, "src2", LN_PARAM_STRING, 0, op_arg);
    check_param(opname, "elew_type", LN_PARAM_STRING, 0, op_arg);
    check_param(opname, "dst", LN_PARAM_STRING, 0, op_arg);

    ln_param_entry *pe;
    pe = ln_param_list_find2(op_arg->params, opname, "elew_type");
    ln_opck_satisfy_msg(str_to_elew_type(pe->value_string) != -1,
                        "unsupported elewop type");
}

static void check_concat(char *opname, ln_op_arg *op_arg)
{
    check_param(opname, "src1", LN_PARAM_STRING, 0, op_arg);
    check_param(opname, "src2", LN_PARAM_STRING, 0, op_arg);
    check_param(opname, "dst", LN_PARAM_STRING, 0, op_arg);
    // axis is optional
}

static void check_scale(char *opname, ln_op_arg *op_arg)
{
    check_param(opname, "src", LN_PARAM_STRING, 0, op_arg);
    check_param(opname, "shift", LN_PARAM_STRING, 0, op_arg);
    check_param(opname, "scale", LN_PARAM_STRING, 0, op_arg);
    check_param(opname, "power", LN_PARAM_STRING, 0, op_arg);
    check_param(opname, "dst", LN_PARAM_STRING, 0, op_arg);
    check_param(opname, "scale_mode", LN_PARAM_STRING, 0, op_arg);

    ln_param_entry *pe;
    pe = ln_param_list_find2(op_arg->params, opname, "scale_mode");
    ln_opck_satisfy_msg(str_to_scale_mode(pe->value_string) != -1,
                        "unsupported scale mode");
}

void ln_tensorrt_check_op(ln_op_arg *op_arg)
{
    int tensors_n;
    ln_param_entry *pe;
    ln_tensor_list_entry *tle;
    ln_tensor_entry *te;
    ln_list *l;

    tensors_n = ln_tensor_list_length(op_arg->tensors_in);
    ln_opck_tensors_in_len_gt(tensors_n, 0);

    for (l = op_arg->tensors_in; l; l = l->next) {
        tle = (ln_tensor_list_entry *)l->data;
        if (ln_streqn(tle->arg_name, "src", 3)) {
            te = ln_tensor_table_find(op_arg->tensor_table, tle->name);
            ln_opck_tensor_defined(te, tle->name);
            ln_opck_tensor_mtype_eq(te, LN_MEM_CUDA);
            ln_opck_satisfy_msg(te->tensor->ndim == 4,
                                "\"src*\" should be a 4-dimensional tensor");
            ln_opck(LN_MSG_ERROR, tl_dtype_to_ioTensor_DataType(te->tensor->dtype) != -1,
                    "%s: \"%s\"'s tensor %s have unsupported input tensor dtype %s for building TensorRT %s model",
                    op_arg->optype, op_arg->name, te->name,
                    tl_dtype_name(te->tensor->dtype), ln_tensorrt_version_str());
        } else if (ln_streqn(tle->arg_name, "weight", 6)) {
            te = ln_tensor_table_find(op_arg->tensor_table, tle->name);
            ln_opck_tensor_defined(te, tle->name);
            ln_opck_tensor_mtype_eq(te, LN_MEM_CPU);
            ln_opck_tensor_isstatic(te);
            ln_opck(LN_MSG_ERROR, tl_dtype_to_weight_DataType(te->tensor->dtype) != -1,
                    "%s: \"%s\"'s tensor %s have unsupported weight tensor dtype %s for building TensorRT %s model",
                    op_arg->optype, op_arg->name, te->name,
                    tl_dtype_name(te->tensor->dtype), ln_tensorrt_version_str());
        }
    }

    tensors_n = ln_tensor_list_length(op_arg->tensors_out);
    ln_opck_tensors_out_len_gt(tensors_n, 0);

    for (l = op_arg->tensors_out; l; l = l->next) {
        tle = (ln_tensor_list_entry *)l->data;
        if (ln_streqn(tle->arg_name, "dst", 3)) {
            te = ln_tensor_table_find(op_arg->tensor_table, tle->name);
            ln_opck_tensor_not_defined(te, tle->name);
        }
    }

    for (l = op_arg->params; l; l = l->next) {
        pe = (ln_param_entry *)l->data;
        if (!ln_streqn(pe->arg_name, "op", 2) || ln_next_token(pe->arg_name, '_'))
            continue;
        ln_opck_param_type(pe, LN_PARAM_STRING);
        if (ln_streq(pe->value_string, "conv"))
            check_conv(pe->arg_name, op_arg);
        else if (ln_streq(pe->value_string, "activation"))
            check_activation(pe->arg_name, op_arg);
        else if (ln_streq(pe->value_string, "pooling"))
            check_pooling(pe->arg_name, op_arg);
        else if (ln_streq(pe->value_string, "softmax"))
            check_softmax(pe->arg_name, op_arg);
        else if (ln_streq(pe->value_string, "elew"))
            check_elew(pe->arg_name, op_arg);
        else if (ln_streq(pe->value_string, "concat"))
            check_concat(pe->arg_name, op_arg);
        else if (ln_streq(pe->value_string, "scale"))
            check_scale(pe->arg_name, op_arg);
        else
            ln_opck(LN_MSG_ERROR, 0, "unsupported TensorRT operator");
    }
    pe = ln_param_list_find(op_arg->params, "batch_size");
    ln_opck_param_exist(pe, "batch_size");
    ln_opck_param_type(pe, LN_PARAM_NUMBER);
    ln_opck_satisfy_msg(pe->value_int > 0, "\"batch_size\" should be a positive integer");

    tl_dtype dtype;
    char *arg_name;
    for (l = op_arg->tensors_out; l; l = l->next) {
        tle = (ln_tensor_list_entry *)l->data;
        arg_name = ln_strcat_delim_alloc(tle->arg_name, "shape", '_');
        pe = ln_param_list_find(op_arg->params, arg_name);
        ln_opck_param_exist(pe, arg_name);
        ln_opck_param_type(pe, LN_PARAM_ARRAY_NUMBER);
        ln_opck_param_array_len_gt(pe, 0);
        ln_free(arg_name);

        arg_name = ln_strcat_delim_alloc(tle->arg_name, "dtype", '_');
        pe = ln_param_list_find(op_arg->params, arg_name);
        ln_opck_param_exist(pe, arg_name);
        ln_opck_param_type(pe, LN_PARAM_STRING);
        dtype = tl_dtype_from_str(pe->value_string);
        ln_opck(LN_MSG_ERROR, tl_dtype_to_ioTensor_DataType(dtype) != -1,
                "%s: \"%s\"'s param \"%s\" have unsupported output tensor dtype %s for building TensorRT %s model",
                op_arg->optype, op_arg->name, arg_name, tl_dtype_name(dtype),
                ln_tensorrt_version_str());
        ln_free(arg_name);
    }
}

class Logger : public ILogger
{
public:
    void log(ILogger::Severity severity, const char* msg) override
    {
        // suppress info-level messages
        if (severity == Severity::kINFO) return;

        switch (severity)
            {
            case Severity::kINTERNAL_ERROR: std::cerr << "INTERNAL_ERROR: "; break;
            case Severity::kERROR: std::cerr << "ERROR: "; break;
            case Severity::kWARNING: std::cerr << "WARNING: "; break;
            case Severity::kINFO: std::cerr << "INFO: "; break;
            default: std::cerr << "UNKNOWN: "; break;
            }
        std::cerr << msg << std::endl;
    }
};

static Logger global_logger;
static size_t max_workspace = 1 << 20; // TODO: what's the difference?

static std::map<std::string, Weights> create_weight_map(ln_op_arg *op_arg)
{
    std::map<std::string, Weights> weights;
    ln_tensor_list_entry *tle;
    ln_tensor_entry *te;
    Weights wt;
    ln_list *l;

    for (l = op_arg->tensors_in; l; l = l->next) {
        tle = (ln_tensor_list_entry *)l->data;
        if (!ln_streqn(tle->arg_name, "weight", 6))
            continue;
        te = ln_tensor_table_find(op_arg->tensor_table, tle->name);
        wt.type = (DataType)tl_dtype_to_weight_DataType(te->tensor->dtype);
        wt.values = te->tensor->data;
        wt.count = te->tensor->len;
        weights[te->name] = wt;
    }

    return weights;
}

static void add_conv(INetworkDefinition *network,
                     std::map<std::string, ITensor*> &tensors,
                     std::map<std::string, Weights> &weights,
                     char *opname, ln_op_arg *op_arg)
{
    char *src;
    char *weight;
    char *bias;
    char *dst;
    int group;
    int output_c;
    int *size;
    int *stride;
    int *padding;
    int *dilation;
    ln_param_entry *pe;

    pe = ln_param_list_find2(op_arg->params, opname, "src");
    assert(pe);
    src = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "weight");
    assert(pe);
    weight = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "bias");
    assert(pe);
    bias = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "dst");
    assert(pe);
    dst = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "group");
    assert(pe);
    group = pe->value_int;

    pe = ln_param_list_find2(op_arg->params, opname, "output_c");
    assert(pe);
    output_c = pe->value_int;

    pe = ln_param_list_find2(op_arg->params, opname, "size");
    assert(pe);
    size = pe->value_array_int;

    pe = ln_param_list_find2(op_arg->params, opname, "stride");
    assert(pe);
    stride = pe->value_array_int;

    pe = ln_param_list_find2(op_arg->params, opname, "padding");
    assert(pe);
    padding = pe->value_array_int;

    pe = ln_param_list_find2(op_arg->params, opname, "dilation");
    assert(pe);
    dilation = pe->value_array_int;

    IConvolutionLayer *conv;
    conv = network->addConvolution(*tensors[src], output_c,
                                   DimsHW(size[0], size[1]),
                                   weights[weight],
                                   weights[bias]);
    assert(conv);
    conv->setNbGroups(group);
    conv->setStride(DimsHW(stride[0], stride[1]));
    conv->setPadding(DimsHW(padding[0], padding[1]));
    conv->setDilation(DimsHW(dilation[0], dilation[1]));
    tensors[dst] = conv->getOutput(0);
}

static void add_activation(INetworkDefinition *network,
                           std::map<std::string, ITensor*> &tensors,
                           char *opname, ln_op_arg *op_arg)
{
    char *src;
    char *dst;
    char *activation_type;
    ln_param_entry *pe;

    pe = ln_param_list_find2(op_arg->params, opname, "src");
    assert(pe);
    src = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "dst");
    assert(pe);
    dst = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "activation_type");
    assert(pe);
    activation_type = pe->value_string;

    IActivationLayer *activation;
    activation = network->addActivation(*tensors[src],
                                        (ActivationType)str_to_activation_type(activation_type));
    assert(activation);
    tensors[dst] = activation->getOutput(0);
}

static void add_pooling(INetworkDefinition *network,
                        std::map<std::string, ITensor*> &tensors,
                        char *opname, ln_op_arg *op_arg)
{
    char *src;
    char *dst;
    char *pooling_type;
    int *size;
    int *stride;
    int *padding;
    ln_param_entry *pe;

    pe = ln_param_list_find2(op_arg->params, opname, "src");
    assert(pe);
    src = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "dst");
    assert(pe);
    dst = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "pooling_type");
    assert(pe);
    pooling_type = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "size");
    assert(pe);
    size = pe->value_array_int;

    pe = ln_param_list_find2(op_arg->params, opname, "stride");
    assert(pe);
    stride = pe->value_array_int;

    pe = ln_param_list_find2(op_arg->params, opname, "padding");
    assert(pe);
    padding = pe->value_array_int;

    IPoolingLayer *pooling;
    pooling = network->addPooling(*tensors[src],
                                  (PoolingType)str_to_pooling_type(pooling_type),
                                  DimsHW(size[0], size[1]));
    assert(pooling);
    pooling->setStride(DimsHW(stride[0], stride[1]));
    pooling->setPadding(DimsHW(padding[0], padding[1]));
    tensors[dst] = pooling->getOutput(0);
}

static void add_softmax(INetworkDefinition *network,
                        std::map<std::string, ITensor*> &tensors,
                        char *opname, ln_op_arg *op_arg)
{
    ln_param_entry *pe;
    char *src;
    char *dst;

    pe = ln_param_list_find2(op_arg->params, opname, "src");
    assert(pe);
    src = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "dst");
    assert(pe);
    dst = pe->value_string;

#if NV_TENSORRT_MAJOR >= 4
    int axes;
    pe = ln_param_list_find2(op_arg->params, opname, "axes");
    if (pe)
        axes = pe->value_int;
#endif

    ISoftMaxLayer *softmax;
    softmax = network->addSoftMax(*tensors[src]);
    assert(softmax);
#if NV_TENSORRT_MAJOR >= 4
    if (pe)
        softmax->setAxes(axes);
#endif
    tensors[dst] = softmax->getOutput(0);
}

static void add_elew(INetworkDefinition *network,
                     std::map<std::string, ITensor*> &tensors,
                     char *opname, ln_op_arg *op_arg)
{
    ln_param_entry *pe;
    char *src1;
    char *src2;
    char *dst;
    char *elew_type;

    pe = ln_param_list_find2(op_arg->params, opname, "src1");
    assert(pe);
    src1 = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "src2");
    assert(pe);
    src2 = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "dst");
    assert(pe);
    dst = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "elew_type");
    assert(pe);
    elew_type = pe->value_string;

    IElementWiseLayer *elew;
    elew = network->addElementWise(*tensors[src1], *tensors[src2],
                                   (ElementWiseOperation)str_to_elew_type(elew_type));
    assert(elew);
    tensors[dst] = elew->getOutput(0);
}

// static void print_dims(const Dims &dims)
// {
//     for (int i = 0; i < dims.nbDims; i++)
//         printf("%d ", dims.d[i]);
//     printf("\n");
// }

static void add_concat(INetworkDefinition *network,
                       std::map<std::string, ITensor*> &tensors,
                       char *opname, ln_op_arg *op_arg)
{
    ln_param_entry *pe;
    char *src1;
    char *src2;
    char *dst;

    pe = ln_param_list_find2(op_arg->params, opname, "src1");
    assert(pe);
    src1 = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "src2");
    assert(pe);
    src2 = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "dst");
    assert(pe);
    dst = pe->value_string;

    IConcatenationLayer *concat;
    // print_dims(tensors[src1]->getDimensions());
    // print_dims(tensors[src2]->getDimensions());
    ITensor *concat_tensors[2] = {tensors[src1], tensors[src2]};
    concat = network->addConcatenation(concat_tensors, 2);
    assert(concat);
#if NV_TENSORRT_MAJOR >= 4
    int axis;
    pe = ln_param_list_find2(op_arg->params, opname, "axis");
    if (pe) {
        axis = pe->value_int;
        concat->setAxis(axis);
    }
#endif
    tensors[dst] = concat->getOutput(0);
}

static void add_scale(INetworkDefinition *network,
                      std::map<std::string, ITensor*> &tensors,
                      std::map<std::string, Weights> &weights,
                      char *opname, ln_op_arg *op_arg)
{
    char *src;
    char *shift;
    char *scale;
    char *power;
    char *dst;
    char *scale_mode;
    ln_param_entry *pe;

    pe = ln_param_list_find2(op_arg->params, opname, "src");
    assert(pe);
    src = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "dst");
    assert(pe);
    dst = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "shift");
    assert(pe);
    shift = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "scale");
    assert(pe);
    scale = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "power");
    assert(pe);
    power = pe->value_string;

    pe = ln_param_list_find2(op_arg->params, opname, "scale_mode");
    assert(pe);
    scale_mode = pe->value_string;

    IScaleLayer *scale_layer;
    scale_layer = network->addScale(*tensors[src], (ScaleMode)str_to_scale_mode(scale_mode), weights[shift], weights[scale], weights[power]);
    assert(scale_layer);
    tensors[dst] = scale_layer->getOutput(0);
}

static ICudaEngine *create_engine(ln_op_arg *op_arg)
{
    IBuilder *builder = createInferBuilder(global_logger);
    INetworkDefinition *network = builder->createNetwork();
    std::map<std::string, Weights> weights = create_weight_map(op_arg);
    std::map<std::string, ITensor*> tensors;

    ln_tensor_entry *te;
    ln_tensor_list_entry *tle;
    ln_list *l;
    DataType dt;
    for (l = op_arg->tensors_in; l; l = l->next) {
        tle = (ln_tensor_list_entry *)l->data;
        if (!ln_streqn(tle->arg_name, "src", 3))
            continue;
        te = ln_tensor_table_find(op_arg->tensor_table, tle->name);
        dt = (DataType)tl_dtype_to_ioTensor_DataType(te->tensor->dtype);
        tensors[te->name] = network->addInput(te->name, dt,
                                              DimsNCHW(te->tensor->dims[0],
                                                       te->tensor->dims[1],
                                                       te->tensor->dims[2],
                                                       te->tensor->dims[3]));
        assert(tensors[te->name]);
        ln_msg_debug("%s: addInput(%s)", op_arg->name, te->name);
    }

    ln_param_entry *pe;
    for (l = op_arg->params; l; l = l->next) {
        pe = (ln_param_entry *)l->data;
        if (!ln_streqn(pe->arg_name, "op", 2) ||
            ln_next_token(pe->arg_name, '_'))
            continue;
        if (ln_streq(pe->value_string, "conv"))
            add_conv(network, tensors, weights, pe->arg_name, op_arg);
        else if (ln_streq(pe->value_string, "activation"))
            add_activation(network, tensors, pe->arg_name, op_arg);
        else if (ln_streq(pe->value_string, "pooling"))
            add_pooling(network, tensors, pe->arg_name, op_arg);
        else if (ln_streq(pe->value_string, "softmax"))
            add_softmax(network, tensors, pe->arg_name, op_arg);
        else if (ln_streq(pe->value_string, "elew"))
            add_elew(network, tensors, pe->arg_name, op_arg);
        else if (ln_streq(pe->value_string, "concat"))
            add_concat(network, tensors, pe->arg_name, op_arg);
        else if (ln_streq(pe->value_string, "scale"))
            add_scale(network, tensors, weights, pe->arg_name, op_arg);
        else
            assert(0 && "unsupported TensorRT operator");
    }

    for (l = op_arg->tensors_out; l; l = l->next) {
        tle = (ln_tensor_list_entry *)l->data;
        tensors[tle->name]->setName(tle->name);
        network->markOutput(*tensors[tle->name]);
        ln_msg_debug("%s: markOutput(%s)", op_arg->name, tle->name);
    }
    pe = ln_param_list_find(op_arg->params, "batch_size");
    builder->setMaxBatchSize(pe->value_int);
    builder->setMaxWorkspaceSize(max_workspace);

    ICudaEngine *engine = builder->buildCudaEngine(*network);
    network->destroy();
    builder->destroy();

    return engine;
}

ln_tensorrt_bundle *ln_tensorrt_bundle_create(ln_op_arg *op_arg)
{
    ICudaEngine *engine;
    IExecutionContext *context;
    void **bindings;
    int batch_size;
    ln_tensorrt_bundle *bundle;

    ln_list *l;
    ln_tensor_list_entry *tle;
    ln_tensor_entry *te;
    int index;

    engine = create_engine(op_arg);
    assert(engine);
    context = engine->createExecutionContext();
    assert(context);
    bindings = (void **)ln_alloc(sizeof(void *)*engine->getNbBindings());
    ln_msg_debug("%s: NbBingdings: %d",
                 op_arg->name, engine->getNbBindings());
    for (l = op_arg->tensors_in; l; l = l->next) {
        tle = (ln_tensor_list_entry *)l->data;
        if (!ln_streqn(tle->arg_name, "src", 3))
            continue;
        index = engine->getBindingIndex(tle->name);
        // printf("%s %s %d\n", op_arg->name, tle->name, index);
        assert(index >= 0);
        te = ln_tensor_table_find(op_arg->tensor_table, tle->name);
        bindings[index] = te->tensor->data;
        ln_msg_debug("%s: %s addr: %p", op_arg->name,
                     te->name, te->tensor->data);
    }
    for (l = op_arg->tensors_out; l; l = l->next) {
        tle = (ln_tensor_list_entry *)l->data;
        if (!ln_streqn(tle->arg_name, "dst", 3))
            continue;
        index = engine->getBindingIndex(tle->name);
        assert(index >= 0);
        te = ln_tensor_table_find(op_arg->tensor_table, tle->name);
        bindings[index] = te->tensor->data;
        ln_msg_debug("%s: %s addr: %p", op_arg->name, te->name,
                     te->tensor->data);
    }
    batch_size = engine->getMaxBatchSize(); // TODO: make batch_size flexible?
    ln_msg_debug("%s: batch_size = %d", op_arg->name, batch_size);

    bundle = (ln_tensorrt_bundle *)ln_alloc(sizeof(ln_tensorrt_bundle));
    bundle->engine = engine;
    bundle->context = context;
    bundle->bindings = bindings;
    bundle->batch_size = batch_size;

    return bundle;
}

void ln_tensorrt_bundle_free(ln_tensorrt_bundle *bundle)
{
    if (!bundle)
        return;
    if (bundle->engine) {
        bundle->context->destroy();
        bundle->engine->destroy();
        ln_free(bundle->bindings);
    }
    ln_free(bundle);
}

void ln_tensorrt_bundle_execute(ln_tensorrt_bundle *bundle)
{
    bundle->context->execute(bundle->batch_size, bundle->bindings);
}

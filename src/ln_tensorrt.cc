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

static const char *tensorrt_version_str(void)
{
     if (TENSORRT_VERSION_STR[0] == 0)
          snprintf(TENSORRT_VERSION_STR, 20, "%d.%d.%d",
                   NV_TENSORRT_MAJOR, NV_TENSORRT_MINOR, NV_TENSORRT_PATCH);
     return TENSORRT_VERSION_STR;
}

static void check_param(const char *name1, const char *name2, ln_param_type ptype,
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

void ln_tensorrt_check_op(ln_op_arg *op_arg, ln_error **error)
{
     int tensors_n;
     ln_param_entry *pe;
     ln_tensor_list_entry *tle;
     ln_tensor_entry *te;
     ln_list *l;

     tensors_n = ln_tensor_list_length(op_arg->tensors_in);
     ln_opck_tensor_in_len_gt(tensors_n, 0);

     for (l = op_arg->tensors_in; l; l = l->next) {
           tle = (ln_tensor_list_entry *)l->data;
          if (!strncmp(tle->arg_name, "src", 3)) {
               te = ln_tensor_table_find(op_arg->tensor_table, tle->name);
               ln_opck_tensor_defined(te, tle->name);
               ln_opck_tensor_mtype_eq(te, LN_MEM_CUDA);
               // TODO: add supported for fp16
#if NV_TENSORRT_MAJOR < 4
               ln_opck(LN_ERROR, te->tensor->dtype == TL_FLOAT,
                       "%s: \"%s\"'s tensor %s have unsupported input tensor dtype %s for building TensorRT %s model",
                       op_arg->optype, op_arg->name, te->name,
                       tl_dtype_name(te->tensor->dtype), tensorrt_version_str());
#else
               ln_opck(LN_ERROR, te->tensor->dtype == TL_FLOAT
                       || te->tensor->dtype == TL_INT32,
                       "%s: \"%s\"'s tensor %s have unsupported input tensor dtype %s for building TensorRT %s model",
                       op_arg->optype, op_arg->name, te->name,
                       tl_dtype_name(te->tensor->dtype), tensorrt_version_str());
#endif
          } else if (!strncmp(tle->arg_name, "weight", 6)) {
               te = ln_tensor_table_find(op_arg->tensor_table, tle->name);
               ln_opck_tensor_defined(te, tle->name);
               ln_opck_tensor_mtype_eq(te, LN_MEM_CPU);
               ln_opck_tensor_isstatic(te);
               // TODO: add supported for fp16
#if NV_TENSORRT_MAJOR < 4
               ln_opck(LN_ERROR, te->tensor->dtype == TL_FLOAT
                       || te->tensor->dtype == TL_INT8,
                       "%s: \"%s\"'s tensor %s have unsupported weight tensor dtype %s for building TensorRT %s model",
                       op_arg->optype, op_arg->name, te->name,
                       tl_dtype_name(te->tensor->dtype), tensorrt_version_str());
#else
               ln_opck(LN_ERROR, te->tensor->dtype == TL_FLOAT
                       || te->tensor->dtype == TL_INT32
                       || te->tensor->dtype == TL_INT8,
                       "%s: \"%s\"'s tensor %s have unsupported weight tensor dtype %s for building TensorRT %s model",
                       op_arg->optype, op_arg->name, te->name,
                       tl_dtype_name(te->tensor->dtype), tensorrt_version_str());
#endif
          }
     }

     tensors_n = ln_tensor_list_length(op_arg->tensors_out);
     ln_opck_tensor_out_len_gt(tensors_n, 0);

     for (l = op_arg->tensors_out; l; l = l->next) {
          tle = (ln_tensor_list_entry *)l->data;
          if (!strncmp(tle->arg_name, "dst", 3)) {
               te = ln_tensor_table_find(op_arg->tensor_table, tle->name);
               ln_opck_tensor_not_defined(te, tle->name);
          }
     }

     for (l = op_arg->params; l; l = l->next) {
          pe = (ln_param_entry *)l->data;
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
     pe = ln_param_list_find(op_arg->params, "max_batch");
     ln_opck_param_exist(pe, "max_batch");
     ln_opck_param_type(pe, LN_PARAM_NUMBER);
     ln_opck_param_satisfy_msg(pe->value_int > 0, "\"max_batch\" should be a positive integer");

     /* define output tensor shape, tensor data should be NULL */
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
          ln_opck_param_type(pe, LN_PARAM_ARRAY_STRING);
          dtype = tl_dtype_from_str(pe->value_string);
#if NV_TENSORRT_MAJOR < 4
          ln_opck(LN_ERROR, dtype == TL_FLOAT,
                  "%s: \"%s\"'s param \"%s\" have unsupported output tensor dtype %s for building TensorRT %s model",
                  op_arg->optype, op_arg->name, arg_name, tl_dtype_name(dtype),
                  tensorrt_version_str());
#else
          ln_opck(LN_ERROR, te->tensor->dtype == TL_FLOAT
                  || te->tensor->dtype == TL_INT32,
                  "%s: \"%s\"'s param \"%s\" have unsupported output tensor dtype %s for building TensorRT %s model",
                  op_arg->optype, op_arg->name, arg_name, tl_dtype_name(dtype),
                  tensorrt_version_str());
#endif
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
          if (strncmp(tle->arg_name, "weight", 6))
               continue;
          te = ln_tensor_table_find(op_arg->tensor_table, tle->name);
          switch (te->tensor->dtype) {
          case TL_FLOAT:
               wt.type = DataType::kFLOAT;
               break;
#if NV_TENSORRT_MAJOR >= 4
          case TL_INT32:
               wt.type = DataType::kINT32;
               break;
#endif
          case TL_INT8:
               wt.type = DataType::kINT8;
               break;
          default:
               assert(0 && "unsupported tensor dtype");
          }
          wt.values = te->tensor->data;
          wt.count = te->tensor->len;
          weights[te->name] = wt;
     }

     return weights;
}

static ICudaEngine *create_engine(ln_op_arg *op_arg, IBuilder *builder)
{
     INetworkDefinition *network = builder->createNetwork();
     std::map<std::string, Weights> weights = create_weight_map(op_arg);
     ln_tensor_entry *te;
     ln_tensor_list_entry *tle;
     ITensor *last_itensor;
     ln_list *l;

     for (l = op_arg->tensors_in; l; l = l->next) {
          tle = (ln_tensor_list_entry *)l->data;
          if (strncmp(tle->arg_name, "src", 3))
               continue;
          te = ln_tensor_table_find(op_arg->tensor_table, tle->name);
     }
}

ln_tensorrt_bundle *ln_tensorrt_bundle_create(ln_op_arg *op_arg)
{
     ICudaEngine *engine;
     IBuilder *builder;

     builder = createInferBuilder(global_logger);

     ln_tensorrt_bundle *bundle;
     bundle = (ln_tensorrt_bundle *)ln_alloc(sizeof(ln_tensorrt_bundle));
     bundle->engine = engine;

     return bundle;
}

void ln_tensorrt_bundle_free(ln_tensorrt_bundle *bundle)
{
     ln_free(bundle);
}

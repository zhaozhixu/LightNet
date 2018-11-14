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

extern ln_op ln_opimpl_tensorrt;
/* end of declare tensorrt ops */

static ln_op *ops_tensorrt[] = {
     &ln_opimpl_tensorrt,
     NULL
};

/* TODO: use hash? */
static inline int can_replace_trt(const char *optype)
{
     if (!strcmp(optype, "create") ||
         !strcmp(optype, "conv2d") ||
         !strcmp(optype, "maxpool2d") ||
         !strcmp(optype, "maxreduce") ||
         !strcmp(optype, "relu") ||
         !strcmp(optype, "reshape") ||
         !strcmp(optype, "slice") ||
         !strcmp(optype, "transpose") ||
         !strcmp(optype, "zeros") ||
         !strcmp(optype, "elew") ||
         !strcmp(optype, "softmax") ||
         !strcmp(optype, "concat"))
          return 1;
     return 0;
}

/* TODO: add a global op name hash */
static ln_op *create_trt_op(ln_op *from_op)
{
     ln_op *trt_op;
     char name[30];

     snprintf(name, 30, "trt%p", from_op);
     trt_op = ln_op_create_from_proto(&ln_opimpl_tensorrt, name, NULL,
                                      NULL, NULL, from_op->op_arg->tensor_table);
     return trt_op;
}

static char *create_new_opname_in_params(ln_list *params)
{
     ln_param_entry *pe;
     int max_idx = 0;
     char *buf = ln_alloc(sizeof(char)*30);

     LN_LIST_FOREACH(pe, params) {
          if (strncmp(pe->arg_name, "op", 2) || ln_next_token(pe->arg_name, '_'))
               continue;
          assert(isdigit(pe->arg_name[2]) && "op subfixed with no digit");
          int idx = atoi(&pe->arg_name[2]);
          max_idx = max_idx < idx ? idx : max_idx;
     }
     snprintf(buf, 30, "op%d", max_idx);
     return buf;
}

static void add_conv_to_trt(ln_op *trt_op, ln_op *op)
{
     ln_param_entry *pe;
     ln_tensor_entry *te;
     ln_tensor_list_entry *tle;
     ln_op_arg *trt_op_arg = trt_op->op_arg;
     ln_op_arg *op_arg = op->op_arg;
     char *opname;
     char *op_arg_name;

     pe = ln_param_list_find(op_arg->params, "padding");
     if (pe->value_array_int[0] != pe->value_array_int[1]
         || pe->value_array_int[2] != pe->value_array_int[3]) {
          ln_error_emit(LN_WARNING, "cannot convert \"\" with asymmetrical padding to TensorRT op",
                        op_arg->name);
          return ;
     }

     opname = create_new_opname_in_params(op_arg->params);
     trt_op_arg->params = ln_param_list_append_string(trt_op_arg->params,
                                                      opname, "conv");

     tle = ln_tensor_list_find_name(op_arg->tensors_in, "src");
     op_arg_name = ln_strcat_delim_alloc(opname, "src", '_');
     trt_op_arg->params = ln_param_list_append_string(trt_op_arg->params,
                                                      op_arg_name, tle->name);
     ln_free(op_arg_name);

     tle = ln_tensor_list_find_name(op_arg->tensors_in, "weight");
     op_arg_name = ln_strcat_delim_alloc(opname, "weight", '_');
     trt_op_arg->params = ln_param_list_append_string(trt_op_arg->params,
                                                      op_arg_name, tle->name);
     ln_free(op_arg_name);

     tle = ln_tensor_list_find_name(op_arg->tensors_in, "bias");
     op_arg_name = ln_strcat_delim_alloc(opname, "bias", '_');
     trt_op_arg->params = ln_param_list_append_string(trt_op_arg->params,
                                                      op_arg_name, tle->name);
     ln_free(op_arg_name);

     tle = ln_tensor_list_find_name(op_arg->tensors_out, "dst");
     op_arg_name = ln_strcat_delim_alloc(opname, "dst", '_');
     trt_op_arg->params = ln_param_list_append_string(trt_op_arg->params,
                                                      op_arg_name, tle->name);
     ln_free(op_arg_name);

     pe = ln_param_list_find(op_arg->params, "group");
     op_arg_name = ln_strcat_delim_alloc(opname, "group", '_');
     trt_op_arg->params = ln_param_list_append_number(trt_op_arg->params,
                                                      op_arg_name, pe->value_double);
     ln_free(op_arg_name);

     tle = ln_tensor_list_find_name(op_arg->tensors_out, "dst");
     te = ln_tensor_table_find(op_arg->tensor_table, tle->name);
     op_arg_name = ln_strcat_delim_alloc(opname, "output_c", '_');
     trt_op_arg->params = ln_param_list_append_number(trt_op_arg->params,
                                                      op_arg_name, te->tensor->dims[1]);
     ln_free(op_arg_name);

     pe = ln_param_list_find(op_arg->params, "size");
     op_arg_name = ln_strcat_delim_alloc(opname, "size", '_');
     trt_op_arg->params = ln_param_list_append_array_number(trt_op_arg->params,
                                                            op_arg_name, 2,
                                                            pe->value_array_double);
     ln_free(op_arg_name);

     pe = ln_param_list_find(op_arg->params, "stride");
     op_arg_name = ln_strcat_delim_alloc(opname, "stride", '_');
     trt_op_arg->params = ln_param_list_append_array_number(trt_op_arg->params,
                                                            op_arg_name, 2,
                                                            pe->value_array_double);
     ln_free(op_arg_name);

     pe = ln_param_list_find(op_arg->params, "padding");
     op_arg_name = ln_strcat_delim_alloc(opname, "padding", '_');
     trt_op_arg->params = ln_param_list_append_array_number(trt_op_arg->params,
                                                            op_arg_name, 2,
                                                            (double[]){pe->value_array_double[0],
                                                                      pe->value_array_double[2]});
     ln_free(op_arg_name);

     pe = ln_param_list_find(op_arg->params, "dilation");
     op_arg_name = ln_strcat_delim_alloc(opname, "dilation", '_');
     trt_op_arg->params = ln_param_list_append_array_number(trt_op_arg->params,
                                                            op_arg_name, 2,
                                                            pe->value_array_double);
     ln_free(op_arg_name);

     ln_free(opname);
}

/* static ln_op *add_to_trt(ln_op *trt_op, ln_op *op) */
/* { */
/*      ln_list *l; */
/*      ln_op *op; */

/*      for (l = ops; l; l = l->next, ++*idx) { */
/*           op = l->data; */

/*      } */
/* } */

/* TODO: add data flow graph */
static ln_list *ph_func_tensorrt(ln_list *ops, int win_size, int *match)
{
     ln_op *op;
     ln_op_arg *op_arg;
     ln_list *l;
     int *replace_flag;
     int i;

     *match = 0;
     replace_flag = ln_alloc(sizeof(int) * win_size);
     for (i = 0, l = ops; l; l = l->next, i++) {
          op = l->data;
          if (can_replace_trt(op->op_arg->optype)) {
               replace_flag[i] = 1;
               *match = 1;
               continue;
          }
          if (!strcmp(op->op_arg->optype, "tensorrt")) {
               replace_flag[i] = 2;
               continue;
          }
          replace_flag[i] = 0;
     }
     for (i = 0; i + 1 < win_size; i++) {
          if (replace_flag[i] == 2 && replace_flag[i+1] == 2)
               *match = 1;
     }
     if (!match) {
          ln_free(replace_flag);
          return NULL;
     }

     ln_list *new_ops = NULL;
     ln_op *new_op;
     ln_list *m;
     int j;
     for (i = 0, l = ops; l; l = l->next, i++) {
          op = l->data;
          op_arg = op->op_arg;
          if (!replace_flag[i]) {
               new_op = ln_op_create_from_proto(op, op_arg->name,
                                                op_arg->tensors_in,
                                                op_arg->tensors_out,
                                                op_arg->params,
                                                op_arg->tensor_table);
               new_ops = ln_list_append(new_ops, new_op);
               continue;
          }
          if (replace_flag[i] == 1) {

          }
          if (replace_flag[i] == 2) {
               for (j = i+1, m = l->next; m && replace_flag[j]; j++, m = m->next) {

               }
          }
     }

     ln_free(replace_flag);
     return new_ops;
}

ln_peephole_func ph_funcs_tensorrt[] = {
     ph_func_tensorrt,
     NULL
};

ln_arch ln_arch_tensorrt = {
     .ops = ops_tensorrt,
     .ph_funcs = ph_funcs_tensorrt,
     .arch_name = "tensorrt",
};

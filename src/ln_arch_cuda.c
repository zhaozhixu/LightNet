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

#include "ln_arch.h"
#include "ln_tensorrt.h"

extern ln_op ln_opimpl_create_cuda;
extern ln_op ln_opimpl_conv2d_cuda;
extern ln_op ln_opimpl_maxpool2d_cuda;
extern ln_op ln_opimpl_maxreduce_cuda;
extern ln_op ln_opimpl_relu_cuda;
extern ln_op ln_opimpl_reshape_cuda;
extern ln_op ln_opimpl_slice_cuda;
extern ln_op ln_opimpl_transpose_cuda;
extern ln_op ln_opimpl_zeros_cuda;
extern ln_op ln_opimpl_elew_cuda;
extern ln_op ln_opimpl_softmax_cuda;
extern ln_op ln_opimpl_concat_cuda;
/* end of declare cuda ops */
extern ln_op ln_opimpl_tensorrt;

static ln_op *ops_cuda[] = {
     &ln_opimpl_create_cuda,
     &ln_opimpl_conv2d_cuda,
     &ln_opimpl_maxpool2d_cuda,
     &ln_opimpl_maxreduce_cuda,
     &ln_opimpl_relu_cuda,
     &ln_opimpl_reshape_cuda,
     &ln_opimpl_slice_cuda,
     &ln_opimpl_transpose_cuda,
     &ln_opimpl_zeros_cuda,
     &ln_opimpl_elew_cuda,
     &ln_opimpl_softmax_cuda,
     &ln_opimpl_concat_cuda,
/* end of init cuda ops */
     &ln_opimpl_tensorrt,
     NULL
};

/* TODO: use hash? */
static inline int can_replace(const char *optype)
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

static ln_list *ph_func_single_replace(ln_list *ops, int win_size, int *match)
{
     ln_op *op, *new_op, *op_proto;
     ln_op_arg *op_arg;
     ln_list *new_ops;
     char *optype_cuda;
     int *replace_flag;
     int i;

     *match = 0;
     replace_flag = ln_alloc(sizeof(int) * win_size);
     i = 0;
     LN_LIST_FOREACH(op, ops) {
          if (can_replace(op->op_arg->optype)) {
               replace_flag[i++] = 1;
               *match = 1;
               continue;
          }
          replace_flag[i++] = 0;
     }
     if (!match) {
          ln_free(replace_flag);
          return NULL;
     }

     new_ops = NULL;
     i = 0;
     LN_LIST_FOREACH(op, ops) {
          op_arg = op->op_arg;
          if (!replace_flag[i++]) {
               new_op = ln_op_create_from_proto(op, op_arg->name,
                                                op_arg->tensors_in,
                                                op_arg->tensors_out,
                                                op_arg->params,
                                                op_arg->tensor_table);
          } else {
               optype_cuda = ln_alloc(sizeof(char)*(strlen(op_arg->optype)+10));
               strcpy(optype_cuda, op_arg->optype);
               strcat(optype_cuda, "_cuda");
               op_proto = ln_op_array_find_by_optype(ops_cuda, optype_cuda);
               assert(op_proto && "optype_cuda not found");
               new_op = ln_op_create_from_proto(op_proto, op_arg->name,
                                                op_arg->tensors_in,
                                                op_arg->tensors_out,
                                                op_arg->params,
                                                op_arg->tensor_table);
               ln_free(optype_cuda);
          }
          new_ops = ln_list_append(new_ops, new_op);
     }

     ln_free(replace_flag);
     return new_ops;
}

/* TODO: use hash? */
static inline int can_replace_tensorrt(const char *optype)
{
     if (!strcmp(optype, "create_cuda") ||
         !strcmp(optype, "conv2d_cuda") ||
         !strcmp(optype, "maxpool2d_cuda") ||
         !strcmp(optype, "maxreduce_cuda") ||
         !strcmp(optype, "relu_cuda") ||
         !strcmp(optype, "reshape_cuda") ||
         !strcmp(optype, "slice_cuda") ||
         !strcmp(optype, "transpose_cuda") ||
         !strcmp(optype, "zeros_cuda") ||
         !strcmp(optype, "elew_cuda") ||
         !strcmp(optype, "softmax_cuda") ||
         !strcmp(optype, "concat_cuda"))
          return 1;
     return 0;
}

static void add_conv(ln_op_arg *trt_op_arg, ln_op_arg *op_arg)
{
     ln_param_entry *pe;

     pe = ln_param_list_find(op_arg->params, "padding");
     if (pe->value_array_int[0] != pe->value_array_int[1]
         || pe->value_array_int[2] != pe->value_array_int[3]) {
          ln_error_emit(LN_WARNING, "cannot convert \"\" with asymmetrical padding to TensorRT op",
                        op_arg->name);
          return ;
     }

}

static void add_to_tensorrt(ln_op_arg *trt_op_arg, ln_op_arg *op_arg)
{
     if (!strcmp(op_arg->optype, "conv_cuda")) {
     }
}

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
          if (can_replace_tensorrt(op->op_arg->optype)) {
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
          if (replace_flag[i] == 2 &&
              (replace_flag[i+1] == 1 || replace_flag[i+1] == 2))
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

ln_peephole_func ph_funcs_cuda[] = {
     ph_func_single_replace,
     /* ph_func_tensorrt, */
     NULL
};

ln_arch ln_arch_cuda = {
     .ops = ops_cuda,
     .ph_funcs = ph_funcs_cuda,
     .arch_name = "cuda",
};

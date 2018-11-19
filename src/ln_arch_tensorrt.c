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

/* TODO: use hash? */
static inline int can_replace_trt(ln_op *op)
{
     if (strcmp(op->op_arg->optype, "create") &&
         strcmp(op->op_arg->optype, "conv2d") &&
         strcmp(op->op_arg->optype, "maxpool2d") &&
         strcmp(op->op_arg->optype, "maxreduce") &&
         strcmp(op->op_arg->optype, "relu") &&
         strcmp(op->op_arg->optype, "reshape") &&
         strcmp(op->op_arg->optype, "slice") &&
         strcmp(op->op_arg->optype, "transpose") &&
         strcmp(op->op_arg->optype, "zeros") &&
         strcmp(op->op_arg->optype, "elew") &&
         strcmp(op->op_arg->optype, "softmax") &&
         strcmp(op->op_arg->optype, "concat"))
          return 0;
     return 1;
}

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
          if (!strcmp(tle->name, name))
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

     tensor_name = ln_tensor_list_find_name(op_arg->tensors_out, origin_tensor_arg);
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
                                                      te->tensor->ndim,
                                                      param_arg_name,
                                                      te->tensor->dims);
     ln_free(param_arg_name);

     param_arg_name = ln_strcat_delim_alloc(tensor_arg_name, "dtype", '_');
     trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                   param_arg_name,
                                                   tl_dtype_name(te->tensor->dtype));

     ln_free(param_arg_name);
     ln_free(tensor_arg_name);
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
     char *tensor_arg_name;

     pe = ln_param_list_find(op_arg->params, "padding");
     if (pe->value_array_int[0] != pe->value_array_int[1]
         || pe->value_array_int[2] != pe->value_array_int[3]) {
          ln_error_emit(LN_WARNING, "cannot convert '%s' with asymmetrical padding to TensorRT op",
                        op_arg->name);
          return ;
     }

     opname = create_name_in_params(op_arg->params, "op");
     trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                   opname, "conv");

     add_src(trt_arg, op_arg, opname, "src");
     add_weight(trt_arg, op_arg, opname, "weight");
     add_weight(trt_arg, op_arg, opname, "bias");
     add_dst(trt_arg, op_arg, opname, "dst");

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
     ln_param_entry *pe;
     ln_tensor_entry *te;
     char *tensor_name;
     ln_op_arg *trt_arg = trt_op->op_arg;
     ln_op_arg *op_arg = op->op_arg;
     char *opname;
     char *param_arg_name;
     char *tensor_arg_name;

     if (strcmp(op_arg->optype, "relu") ||
         strcmp(op_arg->optype, "sigmoid") ||
         strcmp(op_arg->optype, "tanh")) {
          ln_error_emit(LN_WARNING, "cannot convert '%s' to TensorRT op",
                        op_arg->name);
          return ;
     }

     opname = create_name_in_params(op_arg->params, "op");
     trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                   opname, "conv");

     tensor_name = ln_tensor_list_find_name(op_arg->tensors_in, "src");
     param_arg_name = ln_strcat_delim_alloc(opname, "src", '_');
     trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                   param_arg_name, tensor_name);
     tensor_arg_name = create_name_in_tensors(trt_arg->tensors_in, "src");
     trt_arg->tensors_in = ln_tensor_list_append(trt_arg->tensors_in,
                                                 tensor_arg_name, tensor_name);
     ln_free(tensor_arg_name);
     ln_free(param_arg_name);

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
          if (can_replace_trt(op)) {
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

     DFG = ln_op_list_gen_DFG(ops, &node_table);
     LN_LIST_FOREACH(op, ops) {
          if (strcmp(op->op_arg->optype, "tensorrt"))
               continue;

          ln_graph_node *node = ln_hash_find(node_table, op->op_arg->name);
          assert(node);
          ln_tensor_list_entry *tle;
          LN_LIST_FOREACH(tle, op->op_arg->tensors_out) {
               if (is_tensor_been_refered(node->edge_nodes, tle->name))
                    continue;
               ln_tensor_table_remove(op->op_arg->tensor_table, tle->name);
               ln_list_remove(op->op_arg->tensors_out, tle);
          }
     }
     ln_graph_free(DFG);
     ln_hash_free(node_table);
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

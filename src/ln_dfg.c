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

#include "ln_dfg.h"

ln_dfg *ln_dfg_create(ln_list *ops)
{
    ln_dfg *dfg;
    ln_op *op;
    ln_op_arg *arg;
    ln_graph_node *node;
    ln_tensor_list_entry *tle;
    ln_tensor_entry *te;
    int ret;

    dfg = ln_alloc(sizeof(ln_dfg));
    dfg->graph = ln_graph_create(ln_direct_cmp, ln_direct_cmp);
    dfg->node_table = ln_hash_create(ln_str_hash, ln_str_cmp, NULL, NULL);
    LN_LIST_FOREACH(op, ops) {
        node = ln_graph_add(dfg->graph, op);
        ret = ln_hash_insert(dfg->node_table, op->op_arg->name, node);
        if (!ret)
            ln_error_emit(LN_INTER_ERROR, "duplicated op name \"%s\"",
                          op->op_arg->name);
    }
    LN_LIST_FOREACH(op, ops) {
        arg = op->op_arg;
        LN_LIST_FOREACH(tle, arg->tensors_in) {
            te = tle->te;
            node = ln_hash_find(dfg->node_table, te->creater);
            assert(node);
            ln_graph_link(dfg->graph, node->data, op, te);
        }
    }

    return dfg;
}

void ln_dfg_free(ln_dfg *dfg)
{
    if (!dfg)
        return;
    ln_graph_free(dfg->graph);
    ln_hash_free(dfg->node_table);
    ln_free(dfg);
}

void ln_dfg_add(ln_dfg *dfg, ln_op *op)
{
    ln_graph_node *node;
    int ret;

    node = ln_graph_add(dfg->graph, op);
    ret = ln_hash_insert(dfg->node_table, op->op_arg->name, node);
    if (!ret)
        ln_error_emit(LN_INTER_ERROR, "duplicated op name \"%s\"",
                      op->op_arg->name);
}

void ln_dfg_link(ln_dfg *dfg, ln_op *op1, ln_op *op2, ln_tensor_entry *te)
{
    if (!ln_tensor_list_find_by_name(op1->op_arg->tensors_out, te->name) ||
        !ln_tensor_list_find_by_name(op2->op_arg->tensors_in, te->name))
        return;
    ln_graph_link(dfg->graph, op1, op2, te);
}

void ln_dfg_unlink(ln_dfg *dfg, ln_op *op1, ln_op *op2, ln_tensor_entry *te)
{
    if (!ln_tensor_list_find_by_name(op1->op_arg->tensors_out, te->name) ||
        !ln_tensor_list_find_by_name(op2->op_arg->tensors_in, te->name))
        return;
    ln_graph_link(dfg->graph, op1, op2, te);
}

static int edge_node_cmp_by_edge(void *data1, void *data2)
{
    ln_graph_edge_node *en1 = data1;

    return en1->edge_data_cmp(data1, data2);
}

ln_op *ln_dfg_next(ln_dfg *dfg, ln_op *op, ln_tensor_entry *te)
{
    ln_graph_node *node;
    ln_graph_edge_node edge_node;
    ln_graph_edge_node *res;

    node = ln_hash_find(dfg->node_table, op->op_arg->name);
    edge_node.edge_data = te;
    res = ln_list_find_custom(node->out_edge_nodes, &edge_node,
                              edge_node_cmp_by_edge);
    return res ? res->node->data : NULL;
}

ln_op *ln_dfg_prev(ln_dfg *dfg, ln_op *op, ln_tensor_entry *te)
{
    ln_graph_node *node;
    ln_graph_edge_node edge_node;
    ln_graph_edge_node *res;

    node = ln_hash_find(dfg->node_table, op->op_arg->name);
    edge_node.edge_data = te;
    res = ln_list_find_custom(node->in_edge_nodes, &edge_node,
                              edge_node_cmp_by_edge);
    return res ? res->node->data : NULL;
}

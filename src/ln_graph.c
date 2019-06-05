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
#include "ln_graph.h"
#include "ln_queue.h"
#include "ln_stack.h"
#include "ln_hash.h"

ln_graph_node *ln_graph_node_create(void *data, ln_cmp_func node_data_cmp)
{
    ln_graph_node *gn;

    gn = (ln_graph_node *)ln_alloc(sizeof(ln_graph_node));
    gn->in_edge_nodes = NULL;
    gn->out_edge_nodes = NULL;
    gn->data = data;
    gn->node_data_cmp = node_data_cmp;
    gn->indegree = 0;
    gn->outdegree = 0;
    return gn;
}

static void free_edge_node_wrapper(void *data)
{
    ln_graph_edge_node_free(data);
}

/* NOTE: should update the whole graph if not called by ln_graph_free */
void ln_graph_node_free(ln_graph_node *node)
{
    ln_list_free_deep(node->in_edge_nodes, free_edge_node_wrapper);
    ln_list_free_deep(node->out_edge_nodes, free_edge_node_wrapper);
    ln_free(node);
}

ln_graph_edge_node *ln_graph_edge_node_create(void *edge_data,
                                              ln_graph_node *node,
                                              ln_cmp_func edge_data_cmp)
{
    ln_graph_edge_node *en;

    en = (ln_graph_edge_node *)ln_alloc(sizeof(ln_graph_edge_node));
    en->edge_data = edge_data;
    en->edge_data_cmp = edge_data_cmp;
    en->node = node;
    return en;
}

void ln_graph_edge_node_free(ln_graph_edge_node *edge_node)
{
    ln_free(edge_node);
}

ln_graph *ln_graph_create(ln_cmp_func node_data_cmp, ln_cmp_func edge_data_cmp)
{
    ln_graph *g;

    g = (ln_graph *)ln_alloc(sizeof(ln_graph));
    g->nodes = NULL;
    g->size = 0;
    g->node_data_cmp = node_data_cmp;
    g->edge_data_cmp = edge_data_cmp;
    return g;
}

static void free_node_wrapper(void *data)
{
    ln_graph_node_free(data);
}

void ln_graph_free(ln_graph *graph)
{
    ln_list_free_deep(graph->nodes, free_node_wrapper);
    ln_free(graph);
}

ln_graph_node *ln_graph_add(ln_graph *graph, void *data)
{
    ln_graph_node *node;

    node = ln_graph_node_create(data, graph->node_data_cmp);
    graph->nodes = ln_list_append(graph->nodes, node);
    graph->size++;
    return node;
}

static int node_cmp(const void *a, const void *b)
{
    const ln_graph_node *gna = a;
    const ln_graph_node *gnb = b;

    return gna->node_data_cmp(gna->data, gnb->data);
}

ln_graph_node *ln_graph_find(ln_graph *graph, void *data)
{
    ln_graph_node n;

    n.data = data;
    n.node_data_cmp = graph->node_data_cmp;
    return ln_list_find_custom(graph->nodes, &n, node_cmp);
}

void ln_graph_link(ln_graph *graph, void *data1, void *data2, void *edge_data)
{
    ln_graph_node *node1;
    ln_graph_node *node2;

    node1 = ln_graph_find(graph, data1);
    node2 = ln_graph_find(graph, data2);
    if (!node1 || !node2)
        return;

    ln_graph_link_node(graph, node1, node2, edge_data);
}

void ln_graph_link_node(ln_graph *graph, ln_graph_node *node1,
                        ln_graph_node *node2, void *edge_data)
{
    ln_graph_edge_node *out_edge_node;
    ln_graph_edge_node *in_edge_node;

    out_edge_node = ln_graph_edge_node_create(edge_data, node2,
                                              graph->edge_data_cmp);
    node1->out_edge_nodes = ln_list_append(node1->out_edge_nodes, out_edge_node);
    node1->outdegree++;
    in_edge_node = ln_graph_edge_node_create(edge_data, node1,
                                             graph->edge_data_cmp);
    node2->in_edge_nodes = ln_list_append(node2->in_edge_nodes, in_edge_node);
    node2->indegree++;
}

static int edge_node_cmp_by_node(const void *p1, const void *p2)
{
    const ln_graph_edge_node *en1 = p1;
    const ln_graph_edge_node *en2 = p2;

    return node_cmp(en1->node, en2->node);
}

static int edge_node_cmp(const void *p1, const void *p2)
{
    const ln_graph_edge_node *en1 = p1;
    const ln_graph_edge_node *en2 = p2;

    if (node_cmp(en1->node, en2->node) == 0 &&
        en1->edge_data_cmp(en1->edge_data, en2->edge_data) == 0)
        return 0;
    return 1;
}

void *ln_graph_unlink(ln_graph *graph, void *data1, void *data2,
                      void *edge_data)
{
    ln_graph_node *node1;
    ln_graph_node *node2;

    /* TODO: inefficient */
    node1 = ln_graph_find(graph, data1);
    node2 = ln_graph_find(graph, data2);
    if (!node1 || !node2)
        return NULL;

    return ln_graph_unlink_node(node1, node2, edge_data);
}

void *ln_graph_unlink_node(ln_graph_node *node1, ln_graph_node *node2,
                          void *edge_data)
{
    ln_graph_edge_node en;
    ln_graph_edge_node *en1;
    ln_graph_edge_node *en2;
    ln_cmp_func cmp;
    void *ret;

    if (edge_data)
        cmp = edge_node_cmp;
    else
        cmp = edge_node_cmp_by_node;

    en.edge_data = edge_data;
    en.node = node2;
    en1 = ln_list_find_custom(node1->out_edge_nodes, &en, cmp);
    if (!en1)
        return NULL;

    en.node = node1;
    en2 = ln_list_find_custom(node2->in_edge_nodes, &en, cmp);
    if (!en2)
        return NULL;
    assert(en1->edge_data == en2->edge_data);
    ret = en1->edge_data;

    node1->outdegree--;
    node1->out_edge_nodes = ln_list_remove_custom_deep(node1->out_edge_nodes,
                                                       en1, cmp,
                                                       free_edge_node_wrapper);
    node2->indegree--;
    node2->in_edge_nodes = ln_list_remove_custom_deep(node2->in_edge_nodes,
                                                      en2, cmp,
                                                      free_edge_node_wrapper);
    return ret;
}

ln_graph *ln_graph_copy(ln_graph *graph)
{
    ln_graph *g;
    ln_graph_node *node;
    ln_graph_edge_node *edge_node;
    void *data1, *data2, *edge_data;

    g = ln_graph_create(graph->node_data_cmp, graph->edge_data_cmp);
    LN_LIST_FOREACH(node, graph->nodes) {
        ln_graph_add(g, node->data);
    }
    LN_LIST_FOREACH(node, graph->nodes) {
        data1 = node->data;
        LN_LIST_FOREACH(edge_node, node->out_edge_nodes) {
            data2 = edge_node->node->data;
            edge_data = edge_node->edge_data;
            ln_graph_link(g, data1, data2, edge_data);
        }
    }

    return g;
}

int ln_graph_num_outlier(ln_graph *graph)
{
    ln_graph_node *node;
    int num_outlier;

    num_outlier = 0;
    LN_LIST_FOREACH(node, graph->nodes) {
        if (node->indegree == 0 && node->outdegree == 0)
            num_outlier++;
    }
    return num_outlier;
}

void ln_graph_free_topsortlist(ln_list *layers)
{
    ln_list *l;

    for (l = layers; l; l = l->next)
        ln_list_free(l->data);
    ln_list_free(layers);
}

/* return the number of layers of sorted data, -1 if graph has a cycle,
   layers of sorted data is returned in layers  */
int ln_graph_topsort(ln_graph *graph, ln_list **layers)
{
    ln_list *res_list;
    ln_list *sub_list;
    ln_queue *queue;
    ln_graph *g;
    ln_graph_node *node;
    ln_graph_edge_node *edge_node;
    void *data1, *data2;
    int node_count;
    int res_size;
    int queue_size;

    queue = ln_queue_create();
    g = ln_graph_copy(graph);
    LN_LIST_FOREACH(node, g->nodes) {
        if (node->indegree == 0)
            ln_queue_enqueue(queue, node);
    }

    node_count = 0;
    res_size = 0;
    res_list = NULL;
    sub_list = NULL;
    while (queue->size != 0) {
        queue_size = queue->size;
        sub_list = NULL;
        while (queue_size-- != 0) {
            node = ln_queue_dequeue(queue);
            node_count++;
            data1 = node->data;
            sub_list = ln_list_append(sub_list, data1);
            for (ln_list *l = node->out_edge_nodes; l;) {
                edge_node = l->data;
                data2 = edge_node->node->data;
                /* this must be here, since unlink will remove l */
                l = l->next;
                ln_graph_unlink(g, data1, data2, edge_node->edge_data);
                if (edge_node->node->indegree == 0)
                    ln_queue_enqueue(queue, edge_node->node);
            }
        }
        res_list = ln_list_append(res_list, sub_list);
        res_size++;
    }

    ln_queue_free(queue);
    ln_graph_free(g);
    *layers = res_list;

    if (node_count != graph->size - ln_graph_num_outlier(graph))
        return -1;	/* graph has a cycle */
    return res_size;
}

static int can_traverse_node(ln_graph_node *node, ln_hash *visited)
{
    ln_graph_edge_node *en;

    LN_LIST_FOREACH(en, node->in_edge_nodes) {
        if (!ln_hash_find(visited, en->node))
            return 0;
    }
    return 1;
}

/* Depth-First Traversal from a node who has no input edge. Visit every node
   after all its previous nodes have been visited. Return the length of
   `res`, or -1 if graph is empty or has a cycle,*/
int ln_graph_dft_after_prev(ln_graph *graph, ln_list **res)
{
    ln_hash *visited;
    ln_list *res_list;
    ln_stack *stack;
    ln_graph_node *node;
    ln_graph_edge_node *en;
    int len;

    len = 0;
    res_list = NULL;
    stack = ln_stack_create();
    visited = ln_hash_create(ln_direct_hash, ln_direct_cmp, NULL, NULL);
    LN_LIST_FOREACH(node, graph->nodes) {
        if (node->indegree == 0)
            ln_stack_push(stack, node);
    }
    if (stack->size == 0)
        goto err;

    while (stack->size != 0) {
        node = ln_stack_pop(stack);
        ln_hash_insert(visited, node, (void *)1);
        res_list = ln_list_append(res_list, node->data);
        len++;
        LN_LIST_FOREACH(en, node->out_edge_nodes) {
            if (ln_hash_find(visited, en->node))
                goto err;
            if (can_traverse_node(en->node, visited))
                ln_stack_push(stack, en->node);
        }
    }
    *res = res_list;
    goto end;

err: /* graph is empty or has a circle */
    len = -1;
    ln_list_free(res_list);

end:
    ln_stack_free(stack);
    ln_hash_free(visited);

    return len;
}

void ln_graph_fprint(FILE *fp, ln_graph *graph, ln_fprint_func print_node,
                     ln_fprint_func print_edge)
{
    ln_graph_node *node;
    ln_graph_edge_node *edge_node;

    LN_LIST_FOREACH(node, graph->nodes) {
        print_node(fp, node->data);
        fprintf(fp, "-> ");
        LN_LIST_FOREACH(edge_node, node->out_edge_nodes) {
            fprintf(fp, "--");
            if (print_edge)
                print_edge(fp, edge_node->edge_data);
            fprintf(fp, "-");
            print_node(fp, edge_node->node->data);
            fprintf(fp, ", ");
        }
        fprintf(fp, "\n");
    }
}

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

#ifndef _LN_GRAPH_H_
#define _LN_GRAPH_H_

#include "ln_list.h"

struct ln_graph_node {
    void        *data;
    ln_list     *out_edge_nodes;  	/* data type is ln_graph_edge_node */
    ln_list     *in_edge_nodes;
    ln_cmp_func  node_data_cmp;
    size_t       indegree;
    size_t       outdegree;
};
typedef struct ln_graph_node ln_graph_node;

struct ln_graph_edge_node {
    void           *edge_data;
    ln_graph_node  *node;
    ln_cmp_func     edge_data_cmp;
};
typedef struct ln_graph_edge_node ln_graph_edge_node;

struct ln_graph {
    size_t       size;
    ln_list     *nodes;	/* data type is ln_graph_node */
    ln_cmp_func  node_data_cmp;
    ln_cmp_func  edge_data_cmp;
};
typedef struct ln_graph ln_graph;

#ifdef __cplusplus
LN_CPPSTART
#endif

/* The graph don't own any of its data, so free it yourself. */
ln_graph_node *ln_graph_node_create(void *data, ln_cmp_func node_data_cmp);
void ln_graph_node_free(ln_graph_node *node);
ln_graph_edge_node *ln_graph_edge_node_create(void *edge_data,
                                              ln_graph_node *node,
                                              ln_cmp_func edge_data_cmp);
void ln_graph_edge_node_free(ln_graph_edge_node *edge_node);
ln_graph *ln_graph_create(ln_cmp_func node_cmp, ln_cmp_func edge_cmp);
void ln_graph_free(ln_graph *graph);
ln_graph_node *ln_graph_add(ln_graph *graph, void *data);
ln_graph_node *ln_graph_find(ln_graph *graph, void *data);
void ln_graph_link(ln_graph *graph, void *data1, void *data2, void *edge_data);
void ln_graph_link_node(ln_graph *graph, ln_graph_node *node1,
                        ln_graph_node *node2, void *edge_data);
/* if edge_data == NULL, only compare data1 and data2 */
void *ln_graph_unlink(ln_graph *graph, void *data1, void *data2,
                      void *edge_data);
void *ln_graph_unlink_node(ln_graph_node *node1, ln_graph_node *node2,
                          void *edge_data);
ln_graph *ln_graph_copy(ln_graph *graph);
int ln_graph_num_outlier(ln_graph *graph);
void ln_graph_free_topsortlist(ln_list *layers);
/* return the number of layers of sorted data, -1 if graph has a cycle,
   layers of sorted data is returned in layers  */
int ln_graph_topsort(ln_graph *graph, ln_list **layers);
/* print_edge can be NULL */
void ln_graph_fprint(FILE *fp, ln_graph *graph, ln_fprint_func print_node,
                     ln_fprint_func print_edge);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif	/* _LN_GRAPH_H_ */

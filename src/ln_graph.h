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

#ifndef _LN_GRAPH_H_
#define _LN_GRAPH_H_

#include <stdio.h>
#include <stdlib.h>
#include "ln_list.h"
#include "ln_util.h"

typedef struct ln_graph_node ln_graph_node;
struct ln_graph_node {
     size_t       indegree;
     size_t       outdegree;
     void        *data;
     ln_list     *adj_nodes;	/* data type ln_graph_node */
};

typedef struct ln_graph ln_graph;
struct ln_graph {
     size_t       size;
     ln_list     *nodes;	/* data type ln_graph_node */
};

#ifdef __cplusplus
LN_CPPSTART
#endif

ln_graph_node *ln_graph_node_create(void *data);
ln_graph *ln_graph_create(void);
void ln_graph_node_free(ln_graph_node *node);
void ln_graph_free(ln_graph *graph);
ln_graph_node *ln_graph_add(ln_graph *graph, void *data);
ln_graph_node *ln_graph_find(ln_graph *graph, void *data);
void ln_graph_link(ln_graph *graph, void *data1, void *data2);
void ln_graph_unlink(ln_graph *graph, void *data1, void *data2);
ln_graph *ln_graph_copy(ln_graph *graph);
int ln_graph_num_outlier(ln_graph *graph);
void ln_graph_free_topsortlist(ln_list *list);
int ln_graph_topsort(ln_graph *graph, ln_list **res);
void ln_graph_fprint(FILE *fp, ln_graph *graph, ln_fprint_func print_func);

#ifdef __cplusplus
}
#endif

#endif	/* _LN_GRAPH_H_ */

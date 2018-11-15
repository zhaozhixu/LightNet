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

#include "ln_graph.h"
#include "ln_queue.h"
#include "ln_util.h"

static int default_cmp(void *a, void *b)
{
     ln_graph_node *gna;
     ln_graph_node *gnb;

     gna = (ln_graph_node *)a;
     gnb = (ln_graph_node *)b;
     return gna->data - gnb->data;
}

ln_graph_node *ln_graph_node_create(void *data)
{
     ln_graph_node *gn;

     gn = (ln_graph_node *)ln_alloc(sizeof(ln_graph_node));
     gn->adj_nodes = NULL;
     gn->data = data;
     gn->indegree = 0;
     gn->outdegree = 0;
     return gn;
}

ln_graph *ln_graph_create(void)
{
     ln_graph *g;

     g = (ln_graph *)ln_alloc(sizeof(ln_graph));
     g->nodes = NULL;
     g->size = 0;
     return g;
}

void ln_graph_node_free(ln_graph_node *node)
{
     ln_list_free(node->adj_nodes);
     ln_free(node);
}

void ln_graph_free(ln_graph *graph)
{
     ln_list *l;

     for (l = graph->nodes; l; l = l->next)
          ln_graph_node_free(l->data); /* free every node */
     ln_list_free(graph->nodes);	      /* free node list */
     ln_free(graph);
}

ln_graph_node *ln_graph_add(ln_graph *graph, void *data)
{
     ln_graph_node *node;

     node = ln_graph_node_create(data);
     graph->nodes = ln_list_append(graph->nodes, node);
     graph->size++;
     return node;
}

ln_graph_node *ln_graph_find(ln_graph *graph, void *data)
{
     ln_graph_node n;

     n.data = data;
     return ln_list_find_custom(graph->nodes, &n, default_cmp);
}

void ln_graph_link(ln_graph *graph, void *data1, void *data2)
{
     ln_graph_node *node1;
     ln_graph_node *node2;

     node1 = ln_graph_find(graph, data1);
     node2 = ln_graph_find(graph, data2);
     if (!node1 || !node2)
          return;

     node1->adj_nodes = ln_list_append(node1->adj_nodes, node2);
     node1->outdegree++;
     node2->indegree++;
}

void ln_graph_unlink(ln_graph *graph, void *data1, void *data2)
{
     ln_graph_node *node1;
     ln_graph_node *node2;

     node1 = ln_graph_find(graph, data1);
     node2 = ln_graph_find(graph, data2);
     if (!node1 || !node2)
          return;

     if (!ln_list_find(node1->adj_nodes, node2))
          return;

     node1->adj_nodes = ln_list_remove(node1->adj_nodes, node2);
     node1->outdegree--;
     node2->indegree--;
}

ln_graph *ln_graph_copy(ln_graph *graph)
{
     ln_graph *g;
     ln_graph_node *node;
     ln_list *nodes;
     ln_list *adjs;
     void *data1, *data2;

     g = ln_graph_create();
     for (nodes = graph->nodes; nodes; nodes = nodes->next) {
          node = nodes->data;
          ln_graph_add(g, node->data);
     }
     for (nodes = graph->nodes; nodes; nodes = nodes->next) {
          node = nodes->data;
          data1 = node->data;
          for (adjs = node->adj_nodes; adjs; adjs = adjs->next) {
               data2 = ((ln_graph_node *)adjs->data)->data;
               ln_graph_link(g, data1, data2);
          }
     }

     return g;
}

int ln_graph_num_outlier(ln_graph *graph)
{
     ln_list *nodes;
     ln_graph_node *node;
     int num_outlier;

     num_outlier = 0;
     for (nodes = graph->nodes; nodes; nodes = nodes->next) {
          node = nodes->data;
          if (node->indegree == 0 && node->outdegree == 0)
               num_outlier++;
     }
     return num_outlier;
}

void ln_graph_free_topsortlist(ln_list *list)
{
     ln_list *l;

     for (l = list; l; l = l->next)
          ln_list_free(l->data);
     ln_list_free(list);
}

int ln_graph_topsort(ln_graph *graph, ln_list **run_list)
{
     ln_list *nodes;
     ln_list *res_list;
     ln_list *sub_list;
     ln_queue *queue;
     ln_graph *g;
     ln_graph_node *node;
     void *data1, *data2;
     int node_count;
     int res_size;
     int queue_size;

     node_count = 0;
     res_size = 0;
     res_list = NULL;
     sub_list = NULL;
     queue = ln_queue_create();
     g = ln_graph_copy(graph);
     for (nodes = g->nodes; nodes; nodes = nodes->next) {
          node = nodes->data;
          if (node->indegree == 0)
               ln_queue_enqueue(queue, node);
     }

     while (queue->size != 0) {
          queue_size = queue->size;
          sub_list = NULL;
          while (queue_size-- != 0) {
               node = ln_queue_dequeue(queue);
               node_count++;
               data1 = node->data;
               sub_list = ln_list_append(sub_list, data1);
               for (nodes = node->adj_nodes; nodes;) {
                    node = nodes->data;
                    data2 = node->data;
                    nodes = nodes->next;
                    ln_graph_unlink(g, data1, data2);
                    if (node->indegree == 0)
                         ln_queue_enqueue(queue, node);
               }
          }
          res_list = ln_list_append(res_list, sub_list);
          res_size++;
     }

     ln_queue_free(queue);
     ln_graph_free(g);
     *run_list = res_list;

     if (node_count != graph->size - ln_graph_num_outlier(graph))
          return -1;	/* graph has a cycle */
     return res_size;
}

void ln_graph_fprint(FILE *fp, ln_graph *graph, ln_fprint_func print_func)
{
     ln_list *nodes;
     ln_list *adjs;
     ln_graph_node *node;
     ln_graph_node *adj;

     for (nodes = graph->nodes; nodes; nodes = nodes->next) {
          node = nodes->data;
          print_func(fp, node->data);
          fprintf(fp, "->");
          for (adjs = node->adj_nodes; adjs; adjs = adjs->next) {
               adj = adjs->data;
               print_func(fp, adj->data);
               fprintf(fp, " ");
          }
          fprintf(fp, "\n");
     }
}

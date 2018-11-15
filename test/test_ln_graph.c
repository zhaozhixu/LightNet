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

#include "test_lightnet.h"
#include "../src/ln_graph.h"

static int *data;
static size_t data_len;
static ln_graph *graph;

static void checked_setup(void)
{
     int i;
     data_len = 5;
     data = ln_alloc(sizeof(int) * data_len);
     for (i = 0; i < data_len; i++) {
          data[i] = i;
     }

     graph = ln_graph_create();
}

static void checked_teardown(void)
{
     ln_free(data);
     ln_graph_free(graph);
}

START_TEST(test_graph_node_create)
{
     ln_graph_node *node;

     node = ln_graph_node_create(&data[0]);
     ck_assert_int_eq(*(int *)node->data, data[0]);
     ck_assert_int_eq(node->indegree, 0);
     ck_assert_int_eq(node->outdegree, 0);
     ck_assert_ptr_eq(node->adj_nodes, NULL);

     ln_graph_node_free(node);
}
END_TEST

START_TEST(test_graph_create)
{
     ln_graph *g;

     g = ln_graph_create();
     ck_assert_int_eq(g->size, 0);
     ck_assert_ptr_eq(g->nodes, NULL);

     ln_graph_free(g);
}
END_TEST

START_TEST(test_graph_add)
{
     ln_graph_node *n;
     ln_list *l;
     int i;

     for (i = 0; i < data_len; i++) {
          n = ln_graph_add(graph, &data[i]);
          ck_assert_int_eq(*(int *)n->data, data[i]);
          ck_assert_int_eq(graph->size, i+1);
     }
     for (i = 0, l = graph->nodes; l; l = l->next, i++)
          ck_assert_int_eq(*(int *)((ln_graph_node *)l->data)->data, data[i]);
}
END_TEST

START_TEST(test_graph_find)
{
     ln_graph_node **n_array;
     ln_graph_node *n;
     int i, num;

     n_array = (ln_graph_node **)ln_alloc(sizeof(ln_graph_node *)*data_len);
     for (i = 0; i < data_len; i++) {
          n_array[i] = ln_graph_add(graph, &data[i]);
          ck_assert_ptr_eq(ln_graph_find(graph, &data[i]), n_array[i]);
     }
     for (i = 0; i < data_len; i++) {
          n = ln_graph_add(graph, &data[i]);
          ck_assert_ptr_ne(ln_graph_find(graph, &data[i]), n);
     }
     for (i = 0; i < data_len; i++)
          ck_assert_ptr_eq(ln_graph_find(graph, &data[i]), n_array[i]);
     ln_free(n_array);

     num = data_len;
     ck_assert_ptr_eq(ln_graph_find(graph, &num), NULL);

     num = -1;
     ck_assert_ptr_eq(ln_graph_find(graph, &num), NULL);
}
END_TEST

START_TEST(test_graph_link_unlink)
{
     int i;

     for (i = 0; i < data_len; i++)
          ln_graph_add(graph, &data[i]);
     ln_graph_link(graph, &data[0], &data[1]);
     ln_graph_link(graph, &data[0], &data[2]);
     ln_graph_link(graph, &data[0], &data[3]);
     ln_graph_link(graph, &data[3], &data[4]);
     ln_graph_link(graph, &data[2], &data[3]);

     ck_assert_int_eq(ln_graph_find(graph, &data[0])->outdegree, 3);
     ck_assert_int_eq(ln_graph_find(graph, &data[0])->indegree, 0);
     ck_assert_int_eq(ln_graph_find(graph, &data[1])->outdegree, 0);
     ck_assert_int_eq(ln_graph_find(graph, &data[1])->indegree, 1);
     ck_assert_int_eq(ln_graph_find(graph, &data[2])->outdegree, 1);
     ck_assert_int_eq(ln_graph_find(graph, &data[2])->indegree, 1);
     ck_assert_int_eq(ln_graph_find(graph, &data[3])->outdegree, 1);
     ck_assert_int_eq(ln_graph_find(graph, &data[3])->indegree, 2);
     ck_assert_int_eq(ln_graph_find(graph, &data[4])->outdegree, 0);
     ck_assert_int_eq(ln_graph_find(graph, &data[4])->indegree, 1);

     ck_assert_ptr_eq(ln_graph_find(graph, &data[0])->adj_nodes->data, ln_graph_find(graph, &data[1]));
     ck_assert_ptr_eq(ln_graph_find(graph, &data[0])->adj_nodes->next->data, ln_graph_find(graph, &data[2]));
     ck_assert_ptr_eq(ln_graph_find(graph, &data[0])->adj_nodes->next->next->data, ln_graph_find(graph, &data[3]));
     ck_assert_ptr_eq(ln_graph_find(graph, &data[3])->adj_nodes->data, ln_graph_find(graph, &data[4]));
     ck_assert_ptr_eq(ln_graph_find(graph, &data[2])->adj_nodes->data, ln_graph_find(graph, &data[3]));

     ln_graph_unlink(graph, &data[0], &data[1]);
     ln_graph_unlink(graph, &data[0], &data[2]);
     ln_graph_unlink(graph, &data[0], &data[3]);
     ln_graph_unlink(graph, &data[0], &data[4]);
     ln_graph_unlink(graph, &data[3], &data[4]);
     ln_graph_unlink(graph, &data[2], &data[3]);

     ck_assert_int_eq(ln_graph_find(graph, &data[0])->outdegree, 0);
     ck_assert_int_eq(ln_graph_find(graph, &data[0])->indegree, 0);
     ck_assert_int_eq(ln_graph_find(graph, &data[1])->outdegree, 0);
     ck_assert_int_eq(ln_graph_find(graph, &data[1])->indegree, 0);
     ck_assert_int_eq(ln_graph_find(graph, &data[2])->outdegree, 0);
     ck_assert_int_eq(ln_graph_find(graph, &data[2])->indegree, 0);
     ck_assert_int_eq(ln_graph_find(graph, &data[3])->outdegree, 0);
     ck_assert_int_eq(ln_graph_find(graph, &data[3])->indegree, 0);
     ck_assert_int_eq(ln_graph_find(graph, &data[4])->outdegree, 0);

     ck_assert_ptr_eq(ln_graph_find(graph, &data[0])->adj_nodes, NULL);
     ck_assert_ptr_eq(ln_graph_find(graph, &data[1])->adj_nodes, NULL);
     ck_assert_ptr_eq(ln_graph_find(graph, &data[2])->adj_nodes, NULL);
     ck_assert_ptr_eq(ln_graph_find(graph, &data[3])->adj_nodes, NULL);
     ck_assert_ptr_eq(ln_graph_find(graph, &data[4])->adj_nodes, NULL);
}
END_TEST

START_TEST(test_graph_copy)
{
     int i;
     ln_graph *g;
     ln_list *l;

     for (i = 0; i < data_len; i++)
          ln_graph_add(graph, &data[i]);
     ln_graph_link(graph, &data[0], &data[1]);
     ln_graph_link(graph, &data[0], &data[2]);
     ln_graph_link(graph, &data[0], &data[3]);
     ln_graph_link(graph, &data[3], &data[4]);
     ln_graph_link(graph, &data[2], &data[3]);

     g = ln_graph_copy(graph);

     ck_assert_int_eq(g->size, graph->size);
     for (i = 0, l = graph->nodes; l; l = l->next, i++)
          ck_assert_int_eq(*(int *)((ln_graph_node *)l->data)->data, data[i]);

     ck_assert_int_eq(ln_graph_find(g, &data[0])->outdegree, 3);
     ck_assert_int_eq(ln_graph_find(g, &data[0])->indegree, 0);
     ck_assert_int_eq(ln_graph_find(g, &data[1])->outdegree, 0);
     ck_assert_int_eq(ln_graph_find(g, &data[1])->indegree, 1);
     ck_assert_int_eq(ln_graph_find(g, &data[2])->outdegree, 1);
     ck_assert_int_eq(ln_graph_find(g, &data[2])->indegree, 1);
     ck_assert_int_eq(ln_graph_find(g, &data[3])->outdegree, 1);
     ck_assert_int_eq(ln_graph_find(g, &data[3])->indegree, 2);
     ck_assert_int_eq(ln_graph_find(g, &data[4])->outdegree, 0);
     ck_assert_int_eq(ln_graph_find(g, &data[4])->indegree, 1);

     ck_assert_ptr_eq(ln_graph_find(g, &data[0])->adj_nodes->data, ln_graph_find(g, &data[1]));
     ck_assert_ptr_eq(ln_graph_find(g, &data[0])->adj_nodes->next->data, ln_graph_find(g, &data[2]));
     ck_assert_ptr_eq(ln_graph_find(g, &data[0])->adj_nodes->next->next->data, ln_graph_find(g, &data[3]));
     ck_assert_ptr_eq(ln_graph_find(g, &data[3])->adj_nodes->data, ln_graph_find(g, &data[4]));
     ck_assert_ptr_eq(ln_graph_find(g, &data[2])->adj_nodes->data, ln_graph_find(g, &data[3]));

     ln_graph_free(g);
}
END_TEST

START_TEST(test_graph_num_outlier)
{
     int i;

     for (i = 0; i < data_len; i++)
          ln_graph_add(graph, &data[i]);
     ln_graph_link(graph, &data[0], &data[2]);
     ln_graph_link(graph, &data[0], &data[3]);
     ln_graph_link(graph, &data[3], &data[4]);
     ln_graph_link(graph, &data[2], &data[3]);
     ck_assert_int_eq(ln_graph_num_outlier(graph), 1);

     ln_graph_link(graph, &data[0], &data[1]);
     ck_assert_int_eq(ln_graph_num_outlier(graph), 0);
}
END_TEST

START_TEST(test_graph_topsort)
{
     ln_list *res;
     ln_list *sub_list;
     ln_list *l;
     int i;
     int res_num;

     for (i = 0; i < data_len; i++)
          ln_graph_add(graph, &data[i]);
     ln_graph_link(graph, &data[0], &data[1]);
     ln_graph_link(graph, &data[0], &data[2]);
     ln_graph_link(graph, &data[0], &data[3]);
     ln_graph_link(graph, &data[3], &data[4]);
     ln_graph_link(graph, &data[2], &data[3]);

     res_num = ln_graph_topsort(graph, &res);
     ck_assert_int_eq(res_num, 4);

     for (l = res, i = 0; i < 4; l = l->next, i++) {
          if (i == 0) {
               sub_list = l->data;
               ck_assert_ptr_eq(sub_list->data, &data[0]);
               ck_assert_ptr_eq(sub_list->next, NULL);
          }
          if (i == 1) {
               sub_list = l->data;
               ck_assert_ptr_eq(sub_list->data, &data[1]);
               ck_assert_ptr_eq(sub_list->next->data, &data[2]);
               ck_assert_ptr_eq(sub_list->next->next, NULL);
          }
          if (i == 2) {
               sub_list = l->data;
               ck_assert_ptr_eq(sub_list->data, &data[3]);
               ck_assert_ptr_eq(sub_list->next, NULL);
          }
          if (i == 3) {
               sub_list = l->data;
               ck_assert_ptr_eq(sub_list->data, &data[4]);
               ck_assert_ptr_eq(sub_list->next, NULL);
          }
     }
     ln_graph_free_topsortlist(res);

     ln_graph_link(graph, &data[3], &data[2]);
     res_num = ln_graph_topsort(graph, &res);
     ck_assert_int_eq(res_num, -1);
     ln_graph_free_topsortlist(res);
}
END_TEST
/* end of tests */

Suite *make_graph_suite(void)
{
     Suite *s;
     TCase *tc_graph;

     s = suite_create("graph");
     tc_graph = tcase_create("graph");
     tcase_add_checked_fixture(tc_graph, checked_setup, checked_teardown);

     tcase_add_test(tc_graph, test_graph_node_create);
     tcase_add_test(tc_graph, test_graph_create);
     tcase_add_test(tc_graph, test_graph_add);
     tcase_add_test(tc_graph, test_graph_find);
     tcase_add_test(tc_graph, test_graph_link_unlink);
     tcase_add_test(tc_graph, test_graph_copy);
     tcase_add_test(tc_graph, test_graph_num_outlier);
     tcase_add_test(tc_graph, test_graph_topsort);
     /* end of adding tests */

     suite_add_tcase(s, tc_graph);

     return s;
}

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

#include "ln_list.h"
#include "ln_util.h"

static inline ln_list_node *ln_list_node_create(void *data, ln_list_node *next)
{
     ln_list_node *node;

     node = ln_alloc(sizeof(ln_list_node));
     node->data = data;
     node->next = next;
     return node;
}

static inline void ln_list_node_free(ln_list_node *node)
{
     ln_free(node);
}

ln_list *ln_list_create(void)
{
     ln_list *l;

     l = ln_alloc(sizeof(ln_list));
     l->len = 0;
     l->head = NULL;
     return l;
}

/* return the list with appended element (a new list if list == NULL) */
ln_list *ln_list_append(ln_list *list, void *data)
{
     ln_list *l;
     ln_list_node *ln;

     if (!list) {
          l = ln_list_create();
          l->head = ln_list_node_create(data, NULL);
          l->len++;
          return l;
     }
     if (!list->head) {
          list->head = ln_list_node_create(data, NULL);
          list->len++;
          return list;
     }
     for (ln = list->head; ln->next; ln = ln->next)
          ;
     ln->next = ln_list_node_create(data, NULL);
     list->len++;
     return list;
}

void ln_list_free(ln_list *list)
{
     ln_list_node *tmp, *ln;

     if (!list)
          return;
     for (ln = list->head; ln;) {
          tmp = ln->next;
          ln_list_node_free(ln);
          ln = tmp;
     }
     ln_free(list);
}

void ln_list_free_deep(ln_list *list, void (*free_func)(void *))
{
     ln_list_node *tmp, *ln;

     if (!list)
          return;
     for (ln = list->head; ln;) {
          tmp = ln->next;
          free_func(ln->data);
          ln_list_node_free(ln);
          ln = tmp;
     }
     ln_free(list);
}

/* return the nth element in list, or NULL if the position is off the end of list */
static inline ln_list_node *ln_list_nth(ln_list *list, int n)
{
     ln_list_node *ln;
     int i;

     if (n < 0 || n >= list->len)
          return NULL;
     for (ln = list->head, i = 0; ln; ln = ln->next, i++)
          if (i == n)
               return ln;
     return NULL;
}

void *ln_list_nth_data(ln_list *list, int n)
{
     ln_list_node *ln;
     ln = ln_list_nth(list, n);
     return ln ? ln->data : NULL;
}

ln_list *ln_list_remove(ln_list *list, void *data)
{
     ln_list_node **lnp;
     ln_list_node *tmp;

     for (lnp = &list->head; *lnp; lnp = &(*lnp)->next) {
          if ((*lnp)->data == data) {
               tmp = *lnp;
               *lnp = tmp->next;
               ln_list_node_free(tmp);
               list->len--;
               break;
          }
     }
     return list;
}

ln_list *ln_list_remove_custom(ln_list *list, void *data, ln_cmp_func cmp)
{
     ln_list_node **lnp;
     ln_list_node *tmp;

     for (lnp = &list->head; *lnp; lnp = &(*lnp)->next) {
          if (cmp((*lnp)->data, data) == 0) {
               tmp = *lnp;
               *lnp = tmp->next;
               ln_list_node_free(tmp);
               list->len--;
               break;
          }
     }
     return list;
}

ln_list *ln_list_remove_nth(ln_list *list, int n)
{
     ln_list_node **lnp;
     ln_list_node *tmp;
     int i;

     if (n < 0 || n >= list->len)
          return list;
     for (i = 0, lnp = &list->head; *lnp; lnp = &(*lnp)->next, i++) {
          if (i == n) {
               tmp = *lnp;
               *lnp = tmp->next;
               ln_list_node_free(tmp);
               list->len--;
               break;
          }
     }
     return list;
}

ln_list *ln_list_remove_nth_deep(ln_list *list, int n,
                                 void (*free_func)(void *))
{
     ln_list_node **lnp;
     ln_list_node *tmp;
     int i;

     if (n < 0 || n >= list->len)
          return list;
     for (i = 0, lnp = &list->head; *lnp; lnp = &(*lnp)->next, i++) {
          if (i == n) {
               tmp = *lnp;
               *lnp = tmp->next;
               free_func(tmp->data);
               ln_list_node_free(tmp);
               list->len--;
               break;
          }
     }
     return list;
}

/*
 * Return the list with inserted element, or NULL if list == NULL.
 * If n < 0, the new element is added as the head of the list.
 * If n >= the old length of the list, the new element is added on
 * to the end of the list.
 * Otherwise, the new element is inserted before the old nth element.
 */
ln_list *ln_list_insert_nth(ln_list *list, void *data, int n)
{
     ln_list_node **lnp;
     ln_list_node *tmp;
     int i;

     if (n < 0)
          return ln_list_insert_nth(list, data, 0);
     if (n >= list->len)
          return ln_list_append(list, data);

     for (i = 0, lnp = &list->head; *lnp; lnp = &(*lnp)->next, i++) {
          if (i == n) {
               tmp = *lnp;
               *lnp = ln_list_node_create(data, tmp);
               list->len++;
               break;
          }
     }

     return list;
}

void *ln_list_find(ln_list *list, void *data)
{
     ln_list_node *ln;

     for (ln = list->head; ln; ln = ln->next)
          if (data == ln->data)
               return ln->data;
     return NULL;
}

void *ln_list_find_custom(ln_list *list, void *data, ln_cmp_func cmp)
{
     ln_list_node *ln;

     for (ln = list->head; ln; ln = ln->next)
          if (cmp(data, ln->data) == 0)
               return ln->data;
     return NULL;
}

/* not used */
static int ln_list_position(ln_list *list, ln_list *llink)
{
     ln_list *l;
     int i;

     for (i = 0, l = list; l; l = l->next, i++)
          if (l == llink)
               return i;
     return -1;
}

int ln_list_index(ln_list *list, void *data)
{
     ln_list_node *ln;
     int i;

     for (i = 0, ln = list->head; ln; ln = ln->next, i++)
          if (ln->data == data)
               return i;
     return -1;
}

int ln_list_index_custom(ln_list *list, void *data, ln_cmp_func cmp)
{
     ln_list_node *ln;
     int i;

     for (i = 0, ln = list->head; ln; ln = ln->next, i++)
          if (cmp(data, ln->data) == 0)
               return i;
     return -1;
}

int ln_list_length(ln_list *list)
{
     return list->len;
}

ln_list *ln_list_from_array_size_t(size_t *array, size_t n)
{
     ln_list *res;
     size_t i;

     res = NULL;
     for (i = 0; i < n; i++)
          res = ln_list_append(res, (void *)array[i]);
     return res;
}

ln_list *ln_list_copy_size_t(ln_list *list)
{
     ln_list *list_cpy;
     ln_list_node *ln;

     list_cpy = NULL;
     for (ln = list->head; ln; ln = ln->next)
          list_cpy = ln_list_append(list_cpy, ln->data);

     return list_cpy;
}

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

/* return the list with appended element (a new list if list == NULL) */
ln_list *ln_list_append(ln_list *list, void *data)
{
     ln_list *l;

     if (!list) {
          l = (ln_list *)ln_alloc(sizeof(ln_list));
          l->data = data;
          l->next = NULL;
          return l;
     }
     for (l = list; l->next; l = l->next)
          ;
     l->next = (ln_list *)ln_alloc(sizeof(ln_list));
     l->next->data = data;
     l->next->next = NULL;
     return list;
}

void ln_list_free(ln_list *list)
{
     ln_list *tmp, *l;

     for (l = list; l;) {
          tmp = l->next;
          ln_free(l);
          l = tmp;
     }
}

void ln_list_free_deep(ln_list *list, void (*free_func)(void *))
{
     ln_list *tmp, *l;

     for (l = list; l;) {
          tmp = l->next;
          free_func(l->data);
          ln_free(l);
          l = tmp;
     }
}

/* return the nth element in list, or NULL if the position is off the end of list */
ln_list *ln_list_nth(ln_list *list, int n)
{
     ln_list *l;
     int pos;

     for (l = list, pos = 0; l; l = l->next, pos++)
          if (pos == n)
               return l;
     return NULL;
}

void *ln_list_nth_data(ln_list *list, int n)
{
     ln_list *l;
     l = ln_list_nth(list, n);
     return l ? l->data : NULL;
}

ln_list *ln_list_remove(ln_list *list, void *data)
{
     ln_list **lp;
     ln_list *tmp;

     for (lp = &list; *lp; lp = &(*lp)->next) {
          if ((*lp)->data == data) {
               tmp = *lp;
               *lp = tmp->next;
               ln_free(tmp);
               break;
          }
     }
     return list;
}

ln_list *ln_list_remove_nth(ln_list *list, int n)
{
     ln_list **lp;
     ln_list *tmp;
     int i;

     for (i = 0, lp = &list; *lp; lp = &(*lp)->next, i++) {
          if (i == n) {
               tmp = *lp;
               *lp = tmp->next;
               ln_free(tmp);
               break;
          }
     }
     return list;
}

/*
 * Return the list with inserted element, or NULL if list == NULL.
 * If the position n is negative or larger or equal than the length
 * of the list, the new element is added on to the end of the list.
 */
ln_list *ln_list_insert_nth(ln_list *list, void *data, int n)
{
     ln_list **lp;
     ln_list *tmp;
     int i;

     if (n < 0)
          return ln_list_append(list, data);

     for (i = 0, lp = &list; *lp; lp = &(*lp)->next, i++) {
          if (i == n) {
               tmp = *lp;
               *lp = (ln_list *)ln_alloc(sizeof(ln_list));
               (*lp)->data = data;
               (*lp)->next = tmp;
               break;
          }
     }

     if (!*lp)
          *lp = ln_list_append(NULL, data);

     return list;
}

void *ln_list_find(ln_list *list, void *data)
{
     ln_list *l;

     for (l = list; l; l = l->next)
          if (data == l->data)
               return l->data;
     return NULL;
}

void *ln_list_find_custom(ln_list *list, void *data, ln_cmp_func cmp)
{
     ln_list *l;

     for (l = list; l; l = l->next)
          if (cmp(data, l->data) == 0)
               return l->data;
     return NULL;
}

int ln_list_position(ln_list *list, ln_list *llink)
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
     ln_list *l;
     int i;

     for (i = 0, l = list; l; l = l->next, i++)
          if (l->data == data)
               return i;
     return -1;
}

int ln_list_index_custom(ln_list *list, void *data, ln_cmp_func cmp)
{
     ln_list *l;
     int i;

     for (i = 0, l = list; l; l = l->next, i++)
          if (cmp(data, l->data) == 0)
               return i;
     return -1;
}

int ln_list_length(ln_list *list)
{
     ln_list *l;
     int len;

     for (len = 0, l = list; l; l = l->next, len++)
          ;
     return len;
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
     ln_list *l;

     list_cpy = NULL;
     for (l = list; l; l = l->next)
          list_cpy = ln_list_append(list_cpy, l->data);

     return list_cpy;
}

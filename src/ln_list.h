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

#ifndef _LN_LIST_H_
#define _LN_LIST_H_

#include "ln_util.h"

typedef struct ln_list_node ln_list_node;

struct ln_list_node {
     void          *data;
     ln_list_node  *next;
};

typedef struct ln_list ln_list;
struct ln_list {
     int           len;
     ln_list_node *head;
};

#ifdef __cplusplus
LN_CPPSTART
#endif

ln_list *ln_list_append(ln_list *list, void *data);
void ln_list_free(ln_list *list);
void ln_list_free_deep(ln_list *list, void (*free_func)(void *));
ln_list *ln_list_nth(ln_list *list, int n);
void *ln_list_nth_data(ln_list *list, int n);
ln_list *ln_list_remove(ln_list *list, void *data);
ln_list *ln_list_remove_nth(ln_list *list, int n);
ln_list *ln_list_remove_nth_deep(ln_list *list, int n,
                                 void (*free_func)(void *));
ln_list *ln_list_remove_custom(ln_list *list, void *data, ln_cmp_func cmp);
ln_list *ln_list_insert_nth(ln_list *list, void *data, int n);
void *ln_list_find(ln_list *list, void *data);
void *ln_list_find_custom(ln_list *list, void *data, ln_cmp_func cmp);
int ln_list_position(ln_list *list, ln_list *llink);
int ln_list_index(ln_list *list, void *data);
int ln_list_index_custom(ln_list *list, void *data, ln_cmp_func cmp);
int ln_list_length(ln_list *list);
ln_list *ln_list_from_array_size_t(size_t *array, size_t n);
ln_list *ln_list_copy_size_t(ln_list *list);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif     /* _LN_LIST_H_ */

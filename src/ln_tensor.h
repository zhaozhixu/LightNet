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

#ifndef _LN_TENSOR_H_
#define _LN_TENSOR_H_

#include "tl_tensor.h"
#include "ln_list.h"
#include "ln_hash.h"

typedef struct ln_tensor_entry ln_tensor_entry;
struct ln_tensor_entry {
     char       *name;
     char       *arg_name;
     tl_tensor  *tensor;
     size_t      offset;
     int         isstatic;
};

#ifdef __cplusplus
LN_CPPSTART
#endif

ln_list *ln_tensor_list_append(ln_list *list, const char *arg_name,
                               const char *name);
void ln_tensor_list_free(ln_list *list);
char *ln_tensor_list_find_name(ln_list *list, char *arg_name);
int ln_tensor_list_length(ln_list *list);
ln_hash *ln_tensor_table_create(void);
int ln_tensor_table_insert(ln_hash *table, char *name, tl_tensor *tensor);
int ln_tensor_table_remove(ln_hash *table, char *name);
ln_tensor_entry *ln_tensor_table_find(ln_hash *table, char *name);
void ln_tensor_table_free(ln_hash *table);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif	/* _LN_TENSOR_H_ */

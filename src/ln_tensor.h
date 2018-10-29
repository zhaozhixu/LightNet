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
#include "ln_mem.h"

struct ln_tensor_list_entry {
     char *name;
     char *arg_name;
};
typedef struct ln_tensor_list_entry ln_tensor_list_entry;

struct ln_tensor_entry {
     char        *name;
     tl_tensor   *tensor;
     char        *owner;         /* owner of the tensor data */
     size_t       offset;
     int          isstatic;
     ln_mem_type  mtype;
};
typedef struct ln_tensor_entry ln_tensor_entry;

#ifdef __cplusplus
LN_CPPSTART
#endif

ln_list *ln_tensor_list_append(ln_list *list, const char *arg_name,
                               const char *name);
void ln_tensor_list_free(ln_list *list);
char *ln_tensor_list_find_name(ln_list *list, char *arg_name);
int ln_tensor_list_length(ln_list *list);

ln_tensor_entry *ln_tensor_entry_create(const char *name, tl_tensor *tensor);
void ln_tensor_entry_free(ln_tensor_entry *entry);
void ln_tensor_entry_free_tensor_too(ln_tensor_entry *entry);
void ln_tensor_entry_set_owner(ln_tensor_entry *entry, ln_hash *tensor_table,
                               char *direct_owner);

/*
 * When removes or deconstructions or insert-different-tensor-with-same-names
 * happen, this table will free the table entry and the tensor,
 * but not the tensor->data. So we should always insert tensors with NULL data.
 */
ln_hash *ln_tensor_table_create(void);
int ln_tensor_table_insert(ln_hash *table, char *name, ln_tensor_entry *entry);
int ln_tensor_table_remove(ln_hash *table, char *name);
ln_tensor_entry *ln_tensor_table_find(ln_hash *table, char *name);
void ln_tensor_table_free(ln_hash *table);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif	/* _LN_TENSOR_H_ */

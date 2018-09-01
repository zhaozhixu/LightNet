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

typedef struct ln_tensor_entry ln_tensor_entry;
struct ln_tensor_entry {
     char       *name;
     char       *arg_name;
     tl_tensor  *tensor;
};

typedef ln_list ln_tensor_table;

#ifdef __cplusplus
LN_CPPSTART
#endif

ln_tensor_table *ln_tensor_table_create(void);
ln_tensor_table *ln_tensor_table_append(ln_tensor_table *table, const char *arg_name,
					const char *name, tl_tensor *tensor);
void ln_tensor_table_free(ln_tensor_table *table);
ln_tensor_entry *ln_tensor_table_find_by_arg_name(ln_tensor_table *table,
						  char *arg_name);
ln_tensor_entry *ln_tensor_table_find_by_name(ln_tensor_table *table,
					      char *name);
int ln_tensor_table_length(ln_tensor_table *table);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif	/* _LN_TENSOR_H_ */

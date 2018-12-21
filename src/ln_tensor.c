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

#include "ln_tensor.h"
#include "ln_util.h"

ln_tensor_list_entry *ln_tensor_list_entry_create(const char *arg_name,
                                                  const char *name)
{
    ln_tensor_list_entry *entry;

    entry = ln_alloc(sizeof(ln_tensor_list_entry));
    entry->arg_name = ln_strdup(arg_name);
    entry->name = ln_strdup(name);
    entry->offset = 0;
    return entry;
}

void ln_tensor_list_entry_free(ln_tensor_list_entry *entry)
{
    ln_free(entry->arg_name);
    ln_free(entry->name);
    ln_free(entry);
}

static void list_entry_free_wrapper(void *entry)
{
    ln_tensor_list_entry_free(entry);
}

ln_list *ln_tensor_list_append(ln_list *list, const char *arg_name,
                               const char *name)
{
    ln_tensor_list_entry *entry;

    entry = ln_tensor_list_entry_create(arg_name, name);
    list = ln_list_append(list, entry);
    return list;
}

void ln_tensor_list_free(ln_list *list)
{
    ln_list_free_deep(list, list_entry_free_wrapper);
}

ln_list *ln_tensor_list_copy(ln_list *list)
{
    ln_list *new_list = NULL;
    ln_tensor_list_entry *entry;

    LN_LIST_FOREACH(entry, list) {
        new_list = ln_tensor_list_append(new_list, entry->arg_name, entry->name);
    }
    return new_list;
}

static int cmp_by_arg_name(const void *data1, const void *data2)
{
    const ln_tensor_list_entry *e1 = data1;
    const ln_tensor_list_entry *e2 = data2;

    return strcmp(e1->arg_name, e2->arg_name);
}

static int cmp_by_name(const void *data1, const void *data2)
{
    const ln_tensor_list_entry *e1 = data1;
    const ln_tensor_list_entry *e2 = data2;

    return strcmp(e1->name, e2->name);
}

char *ln_tensor_list_find_name(ln_list *list, const char *arg_name)
{
    ln_tensor_list_entry cmp_entry;
    ln_tensor_list_entry *result_entry;

    cmp_entry.arg_name = (char *)arg_name;
    result_entry = ln_list_find_custom(list, &cmp_entry, cmp_by_arg_name);
    if (!result_entry)
        return NULL;
    return result_entry->name;
}

ln_tensor_list_entry *ln_tensor_list_find_by_arg_name(ln_list *list,
                                                      const char *arg_name)
{
    ln_tensor_list_entry cmp_entry;

    cmp_entry.arg_name = (char *)arg_name;
    return ln_list_find_custom(list, &cmp_entry, cmp_by_arg_name);
}

ln_tensor_list_entry *ln_tensor_list_find_by_name(ln_list *list,
                                                  const char *name)
{
    ln_tensor_list_entry cmp_entry;

    cmp_entry.name = (char *)name;
    return ln_list_find_custom(list, &cmp_entry, cmp_by_name);
}

int ln_tensor_list_length(ln_list *list)
{
    return ln_list_length(list);
}

ln_tensor_entry *ln_tensor_entry_create(const char *name, tl_tensor *tensor)
{
    ln_tensor_entry *entry;

    entry = ln_alloc(sizeof(ln_tensor_entry));
    entry->name = ln_strdup(name);
    entry->tensor = tensor;
    entry->owner = NULL;
    entry->creater = NULL;
    entry->offset = 0;
    entry->isstatic = 0;
    entry->mtype = LN_MEM_NONE;

    return entry;
}

void ln_tensor_entry_free(ln_tensor_entry *entry)
{
    ln_free(entry->name);
    ln_free(entry->owner);
    ln_free(entry->creater);
    ln_free(entry);
}

void ln_tensor_entry_free_tensor_too(ln_tensor_entry *entry)
{
    ln_free(entry->name);
    tl_tensor_free(entry->tensor);
    ln_free(entry);
}

static void tensor_entry_free_tensor_too_wrapper(void *p)
{
    ln_tensor_entry_free_tensor_too(p);
}

void ln_tensor_entry_set_owner(ln_tensor_entry *entry, ln_hash *tensor_table,
                               char *direct_owner)
{
    ln_tensor_entry *te;

    assert(entry->name != direct_owner);

    te = ln_tensor_table_find(tensor_table, direct_owner);
    while (te->owner)
        te = ln_tensor_table_find(tensor_table, te->owner);
    entry->owner = ln_strdup(te->name);
}

void ln_tensor_entry_set_creater(ln_tensor_entry *entry, const char *creater)
{
    entry->creater = ln_strdup(creater);
}

ln_hash *ln_tensor_table_create(void)
{
    return ln_hash_create(ln_str_hash, ln_str_cmp, NULL,
                          tensor_entry_free_tensor_too_wrapper);
}

int ln_tensor_table_insert(ln_hash *table, ln_tensor_entry *entry)
{
    return ln_hash_insert(table, entry->name, entry);
}

int ln_tensor_table_remove(ln_hash *table, const char *name)
{
    return ln_hash_remove(table, (char *)name);
}

ln_tensor_entry *ln_tensor_table_find(ln_hash *table, const char *name)
{
    return ln_hash_find(table, (char *)name);
}

void ln_tensor_table_free(ln_hash *table)
{
    ln_hash_free(table);
}

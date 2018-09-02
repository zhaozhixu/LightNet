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

#include "ln_hash.h"
#include "ln_util.h"

static int DEFAULT_INIT_CAPACITY = 16;
static float DEFAULT_LOAD_FACTOR = 0.75f;

typedef struct hash_entry hash_entry;
struct hash_entry {
     void       *key;
     void       *value;
     hash_entry *next;
     int         hash;
};

static inline hash_entry *hash_entry_create(void *key, void *value,
                                            int hash, hash_entry *next)
{
     hash_entry *entry = ln_alloc(sizeof(hash_entry));
     entry->key = key;
     entry->value = value;
     entry->hash = hash;
     entry->next = next;
     return entry;
}

static inline void hash_entry_free(hash_entry *entry)
{
     ln_free(entry);
}

static inline void hash_entry_free_kv_too(hash_entry *entry,
                                          ln_free_func free_k,
                                          ln_free_func free_v)
{
     free_k(entry->key);
     free_v(entry->value);
     ln_free(entry);
}

static inline hash_entry *hash_entry_prepend(hash_entry *head, void *key,
                                             void *value, int hash)
{
     return hash_entry_create(key, value, hash, head);
}

struct ln_hash {
     ln_hash_func hash_func;
     ln_cmp_func  cmp_func;
     hash_entry **table;
     int          capacity;
     int          thresh;
};

ln_hash *ln_hash_create(ln_hash_func hash_func, ln_cmp_func cmp_func)
{
     ln_hash *hash = ln_alloc(sizeof(ln_hash));
     hash->hash_func = hash_func;
     hash->cmp_func = cmp_func;
     hash->capacity = DEFAULT_INIT_CAPACITY;
     hash->thresh = (int)DEFAULT_LOAD_FACTOR * DEFAULT_INIT_CAPACITY;
     return hash;
}

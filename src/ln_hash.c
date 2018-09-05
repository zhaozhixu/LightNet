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

#include <string.h>
#include "ln_hash.h"
#include "ln_util.h"

#define MAX_CAPACITY  (1 << 30)

static const int DEFAULT_INIT_CAPACITY = 16;
static const float DEFAULT_LOAD_FACTOR = 0.75f;

typedef struct hash_entry hash_entry;
struct hash_entry {
     void       *key;
     void       *value;
     hash_entry *next;
     uint32_t    hash_value;
};

static hash_entry *hash_entry_create(void *key, void *value,
                                     uint32_t hash_value, hash_entry *next)
{
     hash_entry *entry = ln_alloc(sizeof(hash_entry));
     entry->key = key;
     entry->value = value;
     entry->hash_value = hash_value;
     entry->next = next;
     return entry;
}

static void hash_entry_free_kv_too(hash_entry *entry,
                                   ln_free_func free_k,
                                   ln_free_func free_v)
{
     free_k(entry->key);
     free_v(entry->value);
     ln_free(entry);
}

static void hash_entry_list_free_kv_too(hash_entry *list,
                                        ln_free_func free_k,
                                        ln_free_func free_v)
{
     hash_entry *l, *tmp;

     for (l = list; l;) {
          tmp = l->next;
          hash_entry_free_kv_too(l, free_k, free_v);
          l = tmp;
     }
}

struct ln_hash {
     ln_hash_func  hash_func;
     ln_cmp_func   cmp_func;
     ln_free_func  free_k_func;
     ln_free_func  free_v_func;
     hash_entry  **table;
     float         load_factor;
     int           capacity;
     int           thresh;
     int           size;
};

static void empty_free(void *data)
{
}

ln_hash *ln_hash_create_full(ln_hash_func hash_func, ln_cmp_func cmp_func,
                             ln_free_func free_k_func, ln_free_func free_v_func,
                             int init_capacity, float load_factor)
{
     ln_hash *hash = ln_alloc(sizeof(ln_hash));
     hash->hash_func = hash_func;
     hash->cmp_func = cmp_func;
     hash->free_k_func = free_k_func ? free_k_func : empty_free;
     hash->free_v_func = free_v_func ? free_v_func : empty_free;

     int capacity = 1;
     while (capacity < init_capacity)
          capacity <<= 1;
     hash->capacity = capacity;
     hash->load_factor = load_factor;
     hash->thresh = (int)(load_factor * capacity);
     hash->table = ln_alloc(sizeof(hash_entry *) * capacity);
     memset(hash->table, 0, sizeof(hash_entry *) * capacity);
     hash->size = 0;

     return hash;
}

ln_hash *ln_hash_create(ln_hash_func hash_func, ln_cmp_func cmp_func,
                        ln_free_func free_k_func, ln_free_func free_v_func)
{
     return ln_hash_create_full(hash_func, cmp_func, free_k_func, free_v_func,
                                DEFAULT_INIT_CAPACITY, DEFAULT_LOAD_FACTOR);
}

void ln_hash_free(ln_hash *hash)
{
     for (int i = 0; i < hash->capacity; i++)
          hash_entry_list_free_kv_too(hash->table[i],
                                      hash->free_k_func, hash->free_v_func);
     ln_free(hash->table);
     ln_free(hash);
}

static inline int index_of(uint32_t h, int capacity)
{
     return (int)(h & ((uint32_t)capacity - 1));
}

static void hash_transfer(ln_hash *hash, hash_entry **new_table,
                          int new_capacity)
{
     hash_entry **old_table = hash->table;
     int old_capacity = hash->capacity;
     hash_entry *next;
     int idx;

     for (int i = 0; i < old_capacity; i++) {
          for (hash_entry *e = old_table[i]; e;) {
               next = e->next;
               idx = index_of(e->hash_value, new_capacity);
               e->next = new_table[idx];
               new_table[idx] = e;
               e = next;
          }
     }
}

static void hash_resize(ln_hash *hash, int new_capacity)
{
     if (hash->capacity == MAX_CAPACITY) {
          hash->thresh = INT32_MAX;
          return;
     }

     hash_entry **new_table = ln_alloc(sizeof(hash_entry *) * new_capacity);
     memset(new_table, 0, sizeof(hash_entry *) * new_capacity);
     hash_transfer(hash, new_table, new_capacity);
     ln_free(hash->table);
     hash->table = new_table;
     hash->capacity = new_capacity;
     hash->thresh = (int)(new_capacity * DEFAULT_LOAD_FACTOR);
}

int ln_hash_insert(ln_hash *hash, void *key, void *value)
{
     int hash_value = hash->hash_func(key);
     int idx = index_of(hash_value, hash->capacity);

     for (hash_entry *e = hash->table[idx]; e; e = e->next) {
          if (e->hash_value == hash_value && !hash->cmp_func(key, e->key)) {
               hash->free_k_func(e->key);
               hash->free_v_func(e->value);
               e->key = key;
               e->value = value;
               return 0;
          }
     }

     hash->table[idx] = hash_entry_create(key, value, hash_value,
                                          hash->table[idx]);

     if (hash->size++ >= hash->thresh)
          hash_resize(hash, 2*hash->capacity);
     return 1;
}

void *ln_hash_find(ln_hash *hash, void *key)
{
     int hash_value = hash->hash_func(key);
     int idx = index_of(hash_value, hash->capacity);
     for (hash_entry *e = hash->table[idx]; e; e = e->next) {
          if (e->hash_value == hash_value && !hash->cmp_func(key, e->key))
               return e->value;
     }
     return NULL;
}

/* in case of NULL key */
int ln_hash_find_extended(ln_hash *hash, void *key, void **value)
{
     int hash_value = hash->hash_func(key);
     int idx = index_of(hash_value, hash->capacity);
     for (hash_entry *e = hash->table[idx]; e; e = e->next) {
          if (e->hash_value == hash_value && !hash->cmp_func(key, e->key)) {
               if (value)
                    *value = e->value;
               return 1;
          }
     }
     return 0;
}

int ln_hash_remove(ln_hash *hash, void *key)
{
     int hash_value = hash->hash_func(key);
     int idx = index_of(hash_value, hash->capacity);
     for (hash_entry **ep = &hash->table[idx]; *ep; ep = &(*ep)->next) {
          hash_entry *e = *ep;
          if (e->hash_value == hash_value && !hash->cmp_func(key, e->key)) {
               *ep = e->next;
               hash_entry_free_kv_too(e, hash->free_k_func, hash->free_v_func);
               hash->size--;
               return 1;
          }
     }
     return 0;
}

int ln_hash_size(ln_hash *hash)
{
     return hash->size;
}

uint32_t ln_direct_hash(void *key)
{
     return (uint32_t)(long)key;
}

int ln_direct_cmp(void *p1, void *p2)
{
     return (long)p1 - (long)p2;
}

uint32_t ln_str_hash(void *key)
{
     const char *p;
     uint32_t h = 5381;

     for (p = key; *p != '\0'; p++)
          h = (h << 5) + h + *p;

     return h;
}

int ln_str_cmp(void *p1, void *p2)
{
     return strcmp(p1, p2);
}

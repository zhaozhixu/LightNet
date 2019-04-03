/*
 * Copyright (c) 2018-2019 Zhao Zhixu
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

#include "ln_name.h"

struct name_record {
    char     name_buf[LN_MAX_NAME_LEN];
    uint32_t name_idx;
};

static ln_hash *name_hash;

void ln_name_init(void)
{
    name_hash = ln_hash_create(ln_str_hash, ln_str_cmp, ln_free, ln_free);
}

void ln_name_cleanup(void)
{
    ln_hash_free(name_hash);
}

const char *ln_name_unique(const char *prefix)
{
    struct name_record *nr;
    char *prefix_copy;

    if (!(nr = ln_hash_find(name_hash, (char *)prefix))) {
        nr = ln_alloc(sizeof(struct name_record));
        memset(nr->name_buf, 0, sizeof(nr->name_buf));
        nr->name_idx = 0;
        prefix_copy = ln_strdup(prefix);
        ln_hash_insert(name_hash, prefix_copy, nr);
    }

    if (strlen(prefix) > LN_MAX_NAME_PREFIX)
        ln_err_msg("WARNING: ln_name_create()'s prefix \"%s\" exceeds LN_MAX_NAME_PREFIX limit %d",
                   prefix, LN_MAX_NAME_PREFIX);

    snprintf(nr->name_buf, LN_MAX_NAME_LEN, "%s%u", prefix, nr->name_idx);
    nr->name_idx++;

    if (nr->name_idx == LN_MAX_NAME_IDX)
        ln_err_msg("WARNING: ln_name_create() exceeds LN_MAX_NAME_IDX limit %s",
                   LN_MAX_NAME_IDX);
    return nr->name_buf;
}

void ln_name_set_idx(const char *prefix, uint32_t idx)
{
    struct name_record *nr;
    char *prefix_copy;

    if (!(nr = ln_hash_find(name_hash, (char *)prefix))) {
        nr = ln_alloc(sizeof(struct name_record));
        memset(nr->name_buf, 0, sizeof(nr->name_buf));
        nr->name_idx = 0;
        prefix_copy = ln_strdup(prefix);
        ln_hash_insert(name_hash, prefix_copy, nr);
    }
    nr->name_idx = idx;
}

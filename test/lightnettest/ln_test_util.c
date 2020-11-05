/*
 * Copyright (c) 2018-2020 Zhixu Zhao
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

#include <err.h>
#include <assert.h>
#include "ln_test_util.h"

void *ln_test_alloc(size_t size)
{
    void *p;

    p = calloc(1, size);
    if (p == NULL) {
        err(EXIT_FAILURE, "%s(): calloc(1, %lu) failed", __func__, size);
    }

    return p;
}

void *ln_test_realloc(void *ptr, size_t size)
{
    void *p;

    p = realloc(ptr, size);
    if (p == NULL && size != 0) {
        err(EXIT_FAILURE, "%s(): realloc() failed", __func__);
    }

    return p;
}

char *ln_test_strdup(const char *s)
{
    char *new_s;

    assert(s);
    new_s = strdup(s);
    if (new_s == NULL) {
        err(EXIT_FAILURE, "%s(): strdup(%s) failed", __func__, s);
    }

    return new_s;
}

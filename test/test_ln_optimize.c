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

#include "test_lightnet.h"
#include "../src/ln_optimize.h"

static void setup(void)
{
}

static void teardown(void)
{
}

START_TEST(test_ln_optimize_mem)
{
     ln_hash *mem_pools;
     ln_mem_pool *mp_cpu, *mp_cuda;

     mp_cpu = ln_mem_pool_create(4096, 1);
     mp_cuda = ln_mem_pool_create(4096, 1);
     mem_pools = ln_hash_create(ln_direct_hash, ln_direct_cmp,
                                NULL, ln_mem_pool_free);
     ln_hash_insert(mem_pools, (void *)LN_MEM_CPU, mp_cpu);
     ln_hash_insert(mem_pools, (void *)LN_MEM_CUDA, mp_cuda);

     ln_hash_free(mem_pools);
}
END_TEST
/* end of tests */

Suite *make_optimize_suite(void)
{
     Suite *s;
     TCase *tc_optimize;

     s = suite_create("optimize");
     tc_optimize = tcase_create("optimize");
     tcase_add_checked_fixture(tc_optimize, setup, teardown);

     tcase_add_test(tc_optimize, test_ln_optimize_mem);
     /* end of adding tests */

     suite_add_tcase(s, tc_optimize);

     return s;
}

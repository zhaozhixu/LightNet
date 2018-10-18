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
#include "../src/ln_mem.h"

static void checked_setup(void)
{
}

static void checked_teardown(void)
{
}

START_TEST(test_ln_mem_pool_create)
{
     ln_mem_pool *mem_pool;

     mem_pool = ln_mem_pool_create(4096, 1);
     ck_assert_int_eq(mem_pool->size, 4096);
     ck_assert_int_eq(mem_pool->align_size, 1);
     ck_assert_ptr_ne(mem_pool->mem_blocks, NULL);
     ln_mem_pool_free(mem_pool);
}
END_TEST

START_TEST(test_ln_mem_pool_free)
{
}
END_TEST

START_TEST(test_ln_mem_alloc)
{
     ln_mem_pool *mem_pool;
     size_t addr1, addr2, addr3, addr4, addr5, addr6, addr7, addr8;

     mem_pool = ln_mem_pool_create(4096, 1);
     addr1 = ln_mem_alloc(mem_pool, 10);
     ck_assert_int_eq(ln_mem_exist(mem_pool, addr1), 1);
     ck_assert_int_eq(addr1, 1);
     addr2 = ln_mem_alloc(mem_pool, 8);
     ck_assert_int_eq(addr2, 11);
     addr3 = ln_mem_alloc(mem_pool, 2);
     ck_assert_int_eq(addr3, 19);
     addr4 = ln_mem_alloc(mem_pool, 4);
     ck_assert_int_eq(addr4, 21);
     addr5 = ln_mem_alloc(mem_pool, 6);
     ck_assert_int_eq(addr5, 25);
     ln_mem_free(mem_pool, addr2);
     ck_assert_int_eq(ln_mem_exist(mem_pool, addr2), 0);
     ln_mem_free(mem_pool, addr4);
     addr6 = ln_mem_alloc(mem_pool, 2);
     ck_assert_int_eq(addr6, 21);
     ln_mem_free(mem_pool, addr3);
     addr7 = ln_mem_alloc(mem_pool, 9);
     ck_assert_int_eq(addr7, 11);
     ln_mem_free(mem_pool, addr6);
     addr8 = ln_mem_alloc(mem_pool, 5);
     ck_assert_int_eq(addr8, 20);
     ln_mem_pool_free(mem_pool);

     mem_pool = ln_mem_pool_create(4096, 8);
     addr1 = ln_mem_alloc(mem_pool, 10);
     ck_assert_int_eq(addr1, 8);
     addr2 = ln_mem_alloc(mem_pool, 9);
     ck_assert_int_eq(addr2, 24);
     addr3 = ln_mem_alloc(mem_pool, 2);
     ck_assert_int_eq(addr3, 40);
     addr4 = ln_mem_alloc(mem_pool, 4);
     ck_assert_int_eq(addr4, 48);
     addr5 = ln_mem_alloc(mem_pool, 6);
     ck_assert_int_eq(addr5, 56);
     ln_mem_free(mem_pool, addr2);
     ln_mem_free(mem_pool, addr4);
     addr6 = ln_mem_alloc(mem_pool, 8);
     ck_assert_int_eq(addr6, 48);
     ln_mem_free(mem_pool, addr3);
     addr7 = ln_mem_alloc(mem_pool, 11);
     ck_assert_int_eq(addr7, 24);
     ln_mem_free(mem_pool, addr6);
     addr8 = ln_mem_alloc(mem_pool, 16);
     ck_assert_int_eq(addr8, 40);
     ln_mem_pool_free(mem_pool);
}
END_TEST

START_TEST(test_ln_mem_free)
{
}
END_TEST
/* end of tests */

Suite *make_mem_suite(void)
{
     Suite *s;
     TCase *tc_mem;

     s = suite_create("mem");
     tc_mem = tcase_create("mem");
     tcase_add_checked_fixture(tc_mem, checked_setup, checked_teardown);

     tcase_add_test(tc_mem, test_ln_mem_pool_create);
     tcase_add_test(tc_mem, test_ln_mem_pool_free);
     tcase_add_test(tc_mem, test_ln_mem_alloc);
     tcase_add_test(tc_mem, test_ln_mem_free);
     /* end of adding tests */

     suite_add_tcase(s, tc_mem);

     return s;
}

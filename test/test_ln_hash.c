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
#include "../src/ln_hash.h"

typedef struct test_object test_object;
struct test_object {
     int a;
     int b;
};

static test_object *test_object_create(int a, int b)
{
     test_object *to = ln_alloc(sizeof(test_object));
     to->a = a;
     to->b = b;
     return to;
}

static void test_object_free(void *to)
{
     ln_free((test_object *)to);
}

static void setup(void)
{
}

static void teardown(void)
{
}

START_TEST(test_ln_hash_create)
{
}
END_TEST

START_TEST(test_ln_hash_free)
{
}
END_TEST

START_TEST(test_ln_hash_insert)
{
     ln_hash *hash;
     test_object *to0, *to2, *to3, *to4, *res;

     hash = ln_hash_create_full(ln_direct_hash, ln_direct_cmp,
                                NULL, test_object_free, 4, 0.75);
     to0 = test_object_create(0, 0);
     to2 = test_object_create(2, 2);
     to3 = test_object_create(3, 3);
     to4 = test_object_create(4, 4);

     ln_hash_insert(hash, (void *)0, to0);
     ln_hash_insert(hash, (void *)2, to2);
     ln_hash_insert(hash, (void *)3, to3);
     ln_hash_insert(hash, (void *)4, to4);
     ck_assert_int_eq(ln_hash_size(hash), 4);
     res = ln_hash_find(hash, (void *)0);
     ck_assert_int_eq(res->a, 0);
     ck_assert_int_eq(res->b, 0);
     res = ln_hash_find(hash, (void *)2);
     ck_assert_int_eq(res->a, 2);
     ck_assert_int_eq(res->b, 2);
     res = ln_hash_find(hash, (void *)3);
     ck_assert_int_eq(res->a, 3);
     ck_assert_int_eq(res->b, 3);
     res = ln_hash_find(hash, (void *)4);
     ck_assert_int_eq(res->a, 4);
     ck_assert_int_eq(res->b, 4);
     res = ln_hash_find(hash, (void *)5);
     ck_assert_ptr_eq(res, NULL);

     ck_assert_int_eq(ln_hash_remove(hash, (void *)4), 1);
     res = ln_hash_find(hash, (void *)4);
     ck_assert_ptr_eq(res, NULL);
     ck_assert_int_eq(ln_hash_size(hash), 3);
     ck_assert_int_eq(ln_hash_remove(hash, (void *)0), 1);
     res = ln_hash_find(hash, (void *)0);
     ck_assert_ptr_eq(res, NULL);
     ck_assert_int_eq(ln_hash_size(hash), 2);
     ck_assert_int_eq(ln_hash_remove(hash, (void *)2), 1);
     res = ln_hash_find(hash, (void *)2);
     ck_assert_ptr_eq(res, NULL);
     ck_assert_int_eq(ln_hash_size(hash), 1);
     ck_assert_int_eq(ln_hash_remove(hash, (void *)5), 0);
     ck_assert_int_eq(ln_hash_size(hash), 1);

     to0 = test_object_create(0, 0);
     to2 = test_object_create(2, 2);
     to3 = test_object_create(3, 3);
     to4 = test_object_create(4, 4);

     ln_hash_insert(hash, (void *)0, to0);
     ln_hash_insert(hash, (void *)2, to2);
     ln_hash_insert(hash, (void *)3, to3);
     ln_hash_insert(hash, (void *)4, to4);
     ck_assert_int_eq(ln_hash_size(hash), 4);
     res = ln_hash_find(hash, (void *)0);
     ck_assert_int_eq(res->a, 0);
     ck_assert_int_eq(res->b, 0);
     res = ln_hash_find(hash, (void *)2);
     ck_assert_int_eq(res->a, 2);
     ck_assert_int_eq(res->b, 2);
     res = ln_hash_find(hash, (void *)3);
     ck_assert_int_eq(res->a, 3);
     ck_assert_int_eq(res->b, 3);
     res = ln_hash_find(hash, (void *)4);
     ck_assert_int_eq(res->a, 4);
     ck_assert_int_eq(res->b, 4);

     ln_hash_free(hash);
}
END_TEST

START_TEST(test_ln_hash_find)
{
}
END_TEST

START_TEST(test_ln_hash_remove)
{
}
END_TEST

START_TEST(test_ln_hash_size)
{
}
END_TEST
/* end of tests */

Suite *make_hash_suite(void)
{
     Suite *s;
     TCase *tc_hash;

     s = suite_create("hash");
     tc_hash = tcase_create("hash");
     tcase_add_checked_fixture(tc_hash, setup, teardown);

     tcase_add_test(tc_hash, test_ln_hash_create);
     tcase_add_test(tc_hash, test_ln_hash_free);
     tcase_add_test(tc_hash, test_ln_hash_insert);
     tcase_add_test(tc_hash, test_ln_hash_find);
     tcase_add_test(tc_hash, test_ln_hash_remove);
     tcase_add_test(tc_hash, test_ln_hash_size);
     /* end of adding tests */

     suite_add_tcase(s, tc_hash);

     return s;
}

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

#include <check.h>
#include <tl_check.h>
#include "test_lightnet.h"
#include "../src/ln_hash.h"

typedef struct test_object test_object;
struct test_object {
    int a;
};

static test_object *test_object_create(int a)
{
    test_object *to = ln_alloc(sizeof(test_object));
    to->a = a;
    return to;
}

static void test_object_free(void *to)
{
    ln_free((test_object *)to);
}

static void checked_setup(void)
{
}

static void checked_teardown(void)
{
}

START_TEST(test_ln_hash)
{
    ln_hash *hash;
    test_object *to0, *to2, *to3, *to4, *to6, *res;
    int ret;

    hash = ln_hash_create_full(ln_direct_hash, ln_direct_cmp,
                               NULL, test_object_free, 4, 0.75);
    to0 = test_object_create(0);
    to2 = test_object_create(2);
    to3 = test_object_create(3);
    to4 = test_object_create(4);
    to6 = test_object_create(6);
    ret = ln_hash_insert(hash, (void *)0, to0);
    ck_assert_int_eq(ret, 1);
    ret = ln_hash_insert(hash, (void *)2, to2);
    ck_assert_int_eq(ret, 1);
    ret = ln_hash_insert(hash, (void *)3, to3);
    ck_assert_int_eq(ret, 1);
    ret = ln_hash_insert(hash, (void *)4, to6);
    ck_assert_int_eq(ret, 1);
    res = ln_hash_find(hash, (void *)4);
    ck_assert_int_eq(res->a, 6);
    ret = ln_hash_insert(hash, (void *)4, to4);
    ck_assert_int_eq(ret, 0);
    res = ln_hash_find(hash, (void *)4);
    ck_assert_int_eq(res->a, 4);
    ret = ln_hash_insert(hash, (void *)4, to4);
    ck_assert_int_eq(ret, 1);
    ck_assert_int_eq(ln_hash_size(hash), 4);

    res = ln_hash_find(hash, (void *)0);
    ck_assert_int_eq(res->a, 0);
    res = ln_hash_find(hash, (void *)2);
    ck_assert_int_eq(res->a, 2);
    res = ln_hash_find(hash, (void *)3);
    ck_assert_int_eq(res->a, 3);
    res = ln_hash_find(hash, (void *)4);
    ck_assert_int_eq(res->a, 4);
    res = ln_hash_find(hash, (void *)5);
    ck_assert_ptr_eq(res, NULL);

    ret = ln_hash_remove(hash, (void *)4);
    ck_assert_int_eq(ret, 1);
    res = ln_hash_find(hash, (void *)4);
    ck_assert_ptr_eq(res, NULL);
    ck_assert_int_eq(ln_hash_size(hash), 3);
    ret = ln_hash_remove(hash, (void *)0);
    ck_assert_int_eq(ret, 1);
    res = ln_hash_find(hash, (void *)0);
    ck_assert_ptr_eq(res, NULL);
    ck_assert_int_eq(ln_hash_size(hash), 2);
    ret = ln_hash_remove(hash, (void *)2);
    ck_assert_int_eq(ret, 1);
    res = ln_hash_find(hash, (void *)2);
    ck_assert_ptr_eq(res, NULL);
    ck_assert_int_eq(ln_hash_size(hash), 1);
    ret = ln_hash_remove(hash, (void *)5);
    ck_assert_int_eq(ret, 0);
    ck_assert_int_eq(ln_hash_size(hash), 1);
    ret = ln_hash_remove(hash, (void *)3);
    ck_assert_int_eq(ret, 1);
    res = ln_hash_find(hash, (void *)3);
    ck_assert_ptr_eq(res, NULL);
    ck_assert_int_eq(ln_hash_size(hash), 0);

    to0 = test_object_create(0);
    to2 = test_object_create(2);
    to3 = test_object_create(3);
    to4 = test_object_create(4);
    ln_hash_insert(hash, (void *)0, to0);
    ln_hash_insert(hash, (void *)2, to2);
    ln_hash_insert(hash, (void *)3, to3);
    ln_hash_insert(hash, (void *)4, to4);
    ck_assert_int_eq(ln_hash_size(hash), 4);

    void *origin_key;
    void *void_res;
    ret = ln_hash_find_extended(hash, (void *)0, &origin_key, &void_res);
    ck_assert_int_eq(ret, 1);
    ck_assert_int_eq(((test_object *)void_res)->a, 0);
    ck_assert_int_eq((long)origin_key, 0);
    ret = ln_hash_find_extended(hash, (void *)2, &origin_key, &void_res);
    ck_assert_int_eq(ret, 1);
    ck_assert_int_eq(((test_object *)void_res)->a, 2);
    ck_assert_int_eq((long)origin_key, 2);
    ret = ln_hash_find_extended(hash, (void *)3, &origin_key, &void_res);
    ck_assert_int_eq(ret, 1);
    ck_assert_int_eq(((test_object *)void_res)->a, 3);
    ck_assert_int_eq((long)origin_key, 3);
    ret = ln_hash_find_extended(hash, (void *)4, &origin_key, &void_res);
    ck_assert_int_eq(ret, 1);
    ck_assert_int_eq(((test_object *)void_res)->a, 4);
    ck_assert_int_eq((long)origin_key, 4);
    ret = ln_hash_find_extended(hash, (void *)5, &origin_key, &void_res);
    ck_assert_int_eq(ret, 0);

    ln_hash_free(hash);
}
END_TEST
/* end of tests */

static TCase *make_hash_tcase(void)
{
    TCase *tc;

    tc = tcase_create("hash");
    tcase_add_checked_fixture(tc, checked_setup, checked_teardown);

    tcase_add_test(tc, test_ln_hash);
    /* end of adding tests */

    return tc;
}

void add_hash_record(test_record *record)
{
    test_record_add_suite(record, "hash");
    test_record_add_tcase(record, "hash", "hash", make_hash_tcase);
}

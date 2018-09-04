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

static int cmp_int(void *data1, void *data2)
{
     return (int)data1 - (int)data2;
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

     hash = ln_hash_create(ln_direct_hash, cmp_int, NULL, test_object_free);
}
END_TEST

START_TEST(test_ln_hash_find)
{
}
END_TEST

START_TEST(test_ln_hash_find_extended)
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
     tcase_add_test(tc_hash, test_ln_hash_find_extended);
     tcase_add_test(tc_hash, test_ln_hash_remove);
     tcase_add_test(tc_hash, test_ln_hash_size);
     /* end of adding tests */

     suite_add_tcase(s, tc_hash);

     return s;
}

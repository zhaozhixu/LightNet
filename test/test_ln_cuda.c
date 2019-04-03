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

#include "test_lightnet.h"
#include "../src/arch/ln_cuda.h"

static void checked_setup(void)
{
}

static void checked_teardown(void)
{
}

START_TEST(test_ln_cuda_set_device)
{
    ln_cuda_set_device(0);
    ck_assert_int_eq(ln_cuda_get_device(), 0);
}
END_TEST

START_TEST(test_ln_cuda_get_device)
{
    ln_cuda_set_device(0);
    ck_assert_int_eq(ln_cuda_get_device(), 0);
}
END_TEST

START_TEST(test_ln_is_device_mem)
{
    int *p;

    p = ln_alloc_cuda(10);
    ck_assert_int_eq(ln_is_device_mem(p), 1);
    ln_free_cuda(p);
    p = ln_alloc(10);
    ck_assert_int_eq(ln_is_device_mem(p), 0);
    ln_free(p);
}
END_TEST

START_TEST(test_ln_alloc_cuda)
{
    int *p = ln_alloc_cuda(10);

    ck_assert_int_eq(ln_is_device_mem(p), 1);
    ln_free_cuda(p);
}
END_TEST

START_TEST(test_ln_memset_cuda)
{
    int *p = ln_alloc_cuda(sizeof(int)*5);
    int p_h[5] = {0};

    ln_memset_cuda(p, -1, sizeof(int)*5);
    ln_memcpy_d2h(p_h, p, sizeof(int)*5);
    for (int i = 0; i < 5; i++)
        ck_assert_int_eq(p_h[i], -1);
    ln_free_cuda(p);
}
END_TEST

START_TEST(test_ln_memcpy_h2d)
{
    int *p = ln_alloc_cuda(sizeof(int)*5);
    int p_h[5] = {0, 1, 2, 3, 4};
    int p_d_h[5] = {0};

    ln_memset_cuda(p, 0, sizeof(int)*5);
    ln_memcpy_h2d(p, p_h, sizeof(int)*5);
    ln_memcpy_d2h(p_d_h, p, sizeof(int)*5);
    for (int i = 0; i < 5; i++)
        ck_assert_int_eq(p_d_h[i], i);
    ln_free_cuda(p);
}
END_TEST

START_TEST(test_ln_memcpy_d2h)
{
    int *p = ln_alloc_cuda(sizeof(int)*5);
    int p_h[5] = {0};

    ln_memset_cuda(p, -1, sizeof(int)*5);
    ln_memcpy_d2h(p_h, p, sizeof(int)*5);
    for (int i = 0; i < 5; i++)
        ck_assert_int_eq(p_h[i], -1);
    ln_free_cuda(p);
}
END_TEST

START_TEST(test_ln_memcpy_d2d)
{
    int *p1 = ln_alloc_cuda(sizeof(int)*5);
    int *p2 = ln_alloc_cuda(sizeof(int)*5);
    int p_h[5] = {0};

    ln_memset_cuda(p1, -1, sizeof(int)*5);
    ln_memset_cuda(p2, 0, sizeof(int)*5);
    ln_memcpy_d2d(p2, p1, sizeof(int)*5);
    ln_memcpy_d2h(p_h, p2, sizeof(int)*5);
    for (int i = 0; i < 5; i++)
        ck_assert_int_eq(p_h[i], -1);
    ln_free_cuda(p1);
    ln_free_cuda(p2);
}
END_TEST

START_TEST(test_ln_free_cuda)
{
    int *p = ln_alloc_cuda(10);

    ck_assert_int_eq(ln_is_device_mem(p), 1);
    ln_free_cuda(p);
    ck_assert_int_eq(ln_is_device_mem(p), 0);
}
END_TEST

START_TEST(test_ln_clone_h2d)
{
    int array[] = {0, 1, 2};
    int *array_d, *array_h;
    int i;

    array_d = ln_clone_h2d(array, sizeof(int)*3);
    array_h = ln_clone_d2h(array_d, sizeof(int)*3);
    for (i = 0; i < 3; i++)
        ck_assert_int_eq(array_h[i], i);

    ln_free(array_h);
    ln_free_cuda(array_d);
}
END_TEST

START_TEST(test_ln_clone_d2h)
{
    int array[] = {0, 1, 2};
    int *array_d, *array_h;
    int i;

    array_d = ln_clone_h2d(array, sizeof(int)*3);
    array_h = ln_clone_d2h(array_d, sizeof(int)*3);
    for (i = 0; i < 3; i++)
        ck_assert_int_eq(array_h[i], i);

    ln_free(array_h);
    ln_free_cuda(array_d);
}
END_TEST

START_TEST(test_ln_clone_d2d)
{
    int array[] = {0, 1, 2};
    int *array_d1, *array_d2, *array_h;
    int i;

    array_d1 = ln_clone_h2d(array, sizeof(int)*3);
    array_d2 = ln_clone_d2d(array_d1, sizeof(int)*3);
    array_h = ln_clone_d2h(array_d2, sizeof(int)*3);
    for (i = 0; i < 3; i++)
        ck_assert_int_eq(array_h[i], i);

    ln_free(array_h);
    ln_free_cuda(array_d1);
    ln_free_cuda(array_d2);
}
END_TEST

START_TEST(test_ln_repeat_h2d)
{
    int array[] = {0, 1, 2};
    int *array_d, *array_h;
    int i;

    array_d = ln_repeat_h2d(array, sizeof(int)*3, 3);
    array_h = ln_clone_d2h(array_d, sizeof(int)*3*3);
    for (i = 0; i < 3*3; i++)
        ck_assert_int_eq(array_h[i], i%3);

    ln_free(array_h);
    ln_free_cuda(array_d);
}
END_TEST

START_TEST(test_ln_repeat_d2h)
{
    int array[] = {0, 1, 2};
    int *array_d, *array_h;
    int i;

    array_d = ln_clone_h2d(array, sizeof(int)*3);
    array_h = ln_repeat_d2h(array_d, sizeof(int)*3, 3);
    for (i = 0; i < 3*3; i++)
        ck_assert_int_eq(array_h[i], i%3);

    ln_free(array_h);
    ln_free_cuda(array_d);
}
END_TEST

START_TEST(test_ln_repeat_d2d)
{
    int array[] = {0, 1, 2};
    int *array_d1, *array_d2, *array_h;
    int i;

    array_d1 = ln_clone_h2d(array, sizeof(int)*3);
    array_d2 = ln_repeat_d2d(array_d1, sizeof(int)*3, 3);
    array_h = ln_clone_d2h(array_d2, sizeof(int)*3*3);
    for (i = 0; i < 3*3; i++)
        ck_assert_int_eq(array_h[i], i%3);

    ln_free(array_h);
    ln_free_cuda(array_d1);
    ln_free_cuda(array_d2);
}
END_TEST

START_TEST(test_ln_cuda_stream_create)
{
}
END_TEST

START_TEST(test_ln_cuda_stream_sync)
{
}
END_TEST
/* end of tests */

Suite *make_cuda_suite(void)
{
     Suite *s;
     TCase *tc_cuda;

     s = suite_create("cuda");
     tc_cuda = tcase_create("cuda");
     tcase_add_checked_fixture(tc_cuda, checked_setup, checked_teardown);

     tcase_add_test(tc_cuda, test_ln_cuda_set_device);
     tcase_add_test(tc_cuda, test_ln_cuda_get_device);
     tcase_add_test(tc_cuda, test_ln_is_device_mem);
     tcase_add_test(tc_cuda, test_ln_alloc_cuda);
     tcase_add_test(tc_cuda, test_ln_memset_cuda);
     tcase_add_test(tc_cuda, test_ln_memcpy_h2d);
     tcase_add_test(tc_cuda, test_ln_memcpy_d2h);
     tcase_add_test(tc_cuda, test_ln_memcpy_d2d);
     tcase_add_test(tc_cuda, test_ln_free_cuda);
     tcase_add_test(tc_cuda, test_ln_clone_h2d);
     tcase_add_test(tc_cuda, test_ln_clone_d2h);
     tcase_add_test(tc_cuda, test_ln_clone_d2d);
     tcase_add_test(tc_cuda, test_ln_repeat_h2d);
     tcase_add_test(tc_cuda, test_ln_repeat_d2h);
     tcase_add_test(tc_cuda, test_ln_repeat_d2d);
     tcase_add_test(tc_cuda, test_ln_cuda_stream_create);
     tcase_add_test(tc_cuda, test_ln_cuda_stream_sync);
     /* end of adding tests */

     suite_add_tcase(s, tc_cuda);

     return s;
}

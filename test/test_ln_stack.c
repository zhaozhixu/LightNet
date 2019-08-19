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
#include "../src/ln_stack.h"

static void checked_setup(void)
{
}

static void checked_teardown(void)
{
}

START_TEST(test_ln_stack_push_pop)
{
     ln_stack *s;

     s = ln_stack_create();

     ck_assert_ptr_eq(ln_stack_pop(s), NULL);
     ln_stack_push(s, (void *)1);
     ln_stack_push(s, (void *)2);
     ln_stack_push(s, (void *)3);

     ck_assert_int_eq((size_t)ln_stack_pop(s), 3);
     ck_assert_int_eq((size_t)ln_stack_pop(s), 2);
     ck_assert_int_eq((size_t)ln_stack_pop(s), 1);
     ck_assert_ptr_eq(ln_stack_pop(s), NULL);
     ck_assert_ptr_eq(ln_stack_pop(s), NULL);

     ln_stack_push(s, (void *)1);
     ln_stack_push(s, (void *)2);
     ln_stack_push(s, (void *)3);

     ck_assert_int_eq((size_t)ln_stack_pop(s), 3);
     ck_assert_int_eq((size_t)ln_stack_pop(s), 2);
     ck_assert_int_eq((size_t)ln_stack_pop(s), 1);
     ck_assert_ptr_eq(ln_stack_pop(s), NULL);
     ck_assert_ptr_eq(ln_stack_pop(s), NULL);

     ln_stack_free(s);
}
END_TEST

/* end of tests */

Suite *make_stack_suite(void)
{
     Suite *s;
     TCase *tc_stack;

     s = suite_create("stack");
     tc_stack = tcase_create("stack");
     tcase_add_checked_fixture(tc_stack, checked_setup, checked_teardown);

     tcase_add_test(tc_stack, test_ln_stack_push_pop);
     /* end of adding tests */

     suite_add_tcase(s, tc_stack);

     return s;
}
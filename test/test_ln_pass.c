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
#include "../src/ln_pass.h"

static void setup(void)
{
}

static void teardown(void)
{
}

START_TEST(test_ln_pass_mem)
{
}
END_TEST
/* end of tests */

Suite *make_pass_suite(void)
{
     Suite *s;
     TCase *tc_pass;

     s = suite_create("pass");
     tc_pass = tcase_create("pass");
     tcase_add_checked_fixture(tc_pass, setup, teardown);

     tcase_add_test(tc_pass, test_ln_pass_mem);
     /* end of adding tests */

     suite_add_tcase(s, tc_pass);

     return s;
}

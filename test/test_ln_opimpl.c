/*
 * Copyright (c) 2018-2020 Zhao Zhixu
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
#include <sys/stat.h>
#include <errno.h>
#include "lightnettest/ln_test.h"
#include "../src/ln_op.h"
#include "../src/ln_pass.h"
#include "../src/ln_arch.h"


static void checked_setup(void)
{
}

static void checked_teardown(void)
{
}

LN_TEST_START(test_ln_opimpl_create)
{
    /* ln_op *op; */
    /* ln_list *ops; */
    /* char *json_str; */

    /* json_str = ln_read_text("test_ops.json"); */
}
LN_TEST_END

LN_TEST_TCASE_START(opimpl, checked_setup, checked_teardown)
{
    LN_TEST_ADD_TEST(test_ln_opimpl_create);
}
LN_TEST_TCASE_END

LN_TEST_ADD_TCASE(opimpl);

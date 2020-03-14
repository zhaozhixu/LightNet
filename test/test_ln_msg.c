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
#include <errno.h>
#include "lightnettest/ln_test.h"
#include "../src/ln_msg.h"

static void checked_setup(void)
{
}

static void checked_teardown(void)
{
}

LN_TEST_START(test_ln_msg_create)
{
    ln_msg *error;

    error = ln_msg_create(LN_MSG_ERROR, "test error %s, %d, %.2f", "test_str", 1, 1.2);
    ck_assert_str_eq(error->err_str, "test error test_str, 1, 1.20");
    ck_assert_int_eq(error->level, LN_MSG_ERROR);
    ln_msg_free(error);

    int errsv;
    char err_str[4096];

    strcpy(err_str, "test error test_str, 1, 1.20: ");
    strcat(err_str+strlen(err_str), strerror(ENOENT));
    errsv = errno;
    errno = ENOENT;
    error = ln_msg_create(LN_MSG_ERROR_SYS, "test error %s, %d, %.2f", "test_str", 1, 1.2);
    errno = errsv;
    ck_assert_str_eq(error->err_str, err_str);
    ck_assert_int_eq(error->level, LN_MSG_ERROR_SYS);
    ln_msg_free(error);
}
LN_TEST_END

LN_TEST_START(test_ln_msg_handle)
{
    /* ln_msg *error; */
    /* int errsv; */

    /* error = ln_msg_create(LN_MSG_INFO, "test error %s, %d, %.2f", "test_str", 1, 1.2); */
    /* ln_msg_handle(&error); */
    /* ck_assert_ptr_eq(error, NULL); */

    /* error = ln_msg_create(LN_MSG_ERROR, "test error %s, %d, %.2f", "test_str", 1, 1.2); */
    /* ln_msg_handle(&error); */
    /* ck_assert_ptr_eq(error, NULL); */

    /* errsv = errno; */
    /* errno = ENOENT; */
    /* error = ln_msg_create(LN_MSG_ERROR_SYS, "test error %s, %d, %.2f", "test_str", 1, 1.2); */
    /* errno = errsv; */
    /* ln_msg_handle(&error); */
    /* ck_assert_ptr_eq(error, NULL); */

    /* error = ln_msg_create(LN_MSG_WARN, "test error %s, %d, %.2f", "test_str", 1, 1.2); */
    /* ln_msg_handle(&error); */
    /* ck_assert_ptr_eq(error, NULL); */

    /* errsv = errno; */
    /* errno = ENOENT; */
    /* error = ln_msg_create(LN_MSG_WARN_SYS, "test error %s, %d, %.2f", "test_str", 1, 1.2); */
    /* errno = errsv; */
    /* ln_msg_handle(&error); */
    /* ck_assert_ptr_eq(error, NULL); */
}
LN_TEST_END

LN_TEST_TCASE_START(msg, checked_setup, checked_teardown)
{
    LN_TEST_ADD_TEST(test_ln_msg_create);
    LN_TEST_ADD_TEST(test_ln_msg_handle);
}
LN_TEST_TCASE_END

LN_TEST_ADD_TCASE(msg);

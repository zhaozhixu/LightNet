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

#include <errno.h>
#include "test_lightnet.h"
#include "../src/ln_msg.h"

static void checked_checked_setup(void)
{
}

static void checked_teardown(void)
{
}

START_TEST(test_ln_msg_create)
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
END_TEST

START_TEST(test_ln_msg_handle)
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
END_TEST
/* end of tests */

Suite *make_msg_suite(void)
{
     Suite *s;
     TCase *tc_msg;

     s = suite_create("msg");
     tc_msg = tcase_create("msg");
     tcase_add_checked_fixture(tc_msg, checked_checked_setup, checked_teardown);

     tcase_add_test(tc_msg, test_ln_msg_create);
     tcase_add_test(tc_msg, test_ln_msg_handle);
     /* end of adding tests */

     suite_add_tcase(s, tc_msg);

     return s;
}

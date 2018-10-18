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

#include <sys/stat.h>
#include <errno.h>
#include "test_lightnet.h"
#include "../src/ln_op.h"
#include "../src/ln_pass.h"

static ln_list *reg_ops;
static ln_error *error = NULL;
static ln_hash *tensor_table;

extern ln_op *ln_init_ops[];

static void checked_setup(void)
{
     reg_ops = ln_op_list_create_from_array(ln_init_ops);
     tensor_table = ln_tensor_table_create();
}

static void checked_teardown(void)
{
     ln_tensor_table_free(tensor_table);
     ln_op_list_free(reg_ops);
}

START_TEST(test_ln_opimpl_create)
{
     ln_op *op;
     ln_list *ops;
     char *json_str;

     json_str = ln_read_text("test_ops.json");
}
END_TEST
/* end of tests */

Suite *make_opimpl_suite(void)
{
     Suite *s;
     TCase *tc_opimpl;

     s = suite_create("opimpl");
     tc_opimpl = tcase_create("opimpl");
     tcase_add_checked_fixture(tc_opimpl, checked_setup, checked_teardown);

     tcase_add_test(tc_opimpl, test_ln_opimpl_create);
     /* end of adding tests */

     suite_add_tcase(s, tc_opimpl);

     return s;
}

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

#include <stdlib.h>
#include <stdio.h>
#include <check.h>

#include "test_lightnet.h"

int main(int argc, char **argv)
{
     int number_failed;
     int status;
     SRunner *sr;

     sr = srunner_create(make_master_suite());
     srunner_add_suite(sr, make_list_suite());
     srunner_add_suite(sr, make_hash_suite());
     srunner_add_suite(sr, make_error_suite());
     srunner_add_suite(sr, make_param_suite());
     srunner_add_suite(sr, make_tensor_suite());
     srunner_add_suite(sr, make_op_suite());
     srunner_add_suite(sr, make_mem_suite());
     srunner_add_suite(sr, make_pass_suite());
     /* end of adding suites */

     srunner_set_xml (sr, "result/check_output.xml");
     srunner_run_all(sr, CK_NORMAL);
     number_failed = srunner_ntests_failed(sr);
     srunner_free(sr);
     status = system("sed -i 's,http://check.sourceforge.net/xml/check_unittest.xslt,check_unittest.xslt,g' result/check_output.xml");
     if (status < 0)
          fprintf(stderr, "system() error\n");

     return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

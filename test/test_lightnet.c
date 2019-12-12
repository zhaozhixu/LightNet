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

#include <stdlib.h>
#include <check.h>

#include "test_lightnet.h"

#ifndef LN_TEST_RESULT_DIR
#define LN_TEST_RESULT_DIR "."
#endif

int main(int argc, char **argv)
{
     int number_failed;
     const char *cmd;
     SRunner *sr;

     sr = srunner_create(make_master_suite());
     srunner_add_suite(sr, make_util_suite());
#ifdef LN_CUDA
     srunner_add_suite(sr, make_cuda_suite());
#endif
     srunner_add_suite(sr, make_list_suite());
     srunner_add_suite(sr, make_queue_suite());
     srunner_add_suite(sr, make_stack_suite());
     srunner_add_suite(sr, make_hash_suite());
     srunner_add_suite(sr, make_graph_suite());
     srunner_add_suite(sr, make_msg_suite());
     srunner_add_suite(sr, make_param_suite());
     srunner_add_suite(sr, make_tensor_suite());
     srunner_add_suite(sr, make_op_suite());
     srunner_add_suite(sr, make_parse_suite());
     srunner_add_suite(sr, make_mem_suite());
     srunner_add_suite(sr, make_pass_suite());
     srunner_add_suite(sr, make_opimpl_suite());
     /* end of adding suites */

     srunner_set_xml (sr, LN_TEST_RESULT_DIR"/check_output.xml");
     srunner_run_all(sr, CK_NORMAL);
     number_failed = srunner_ntests_failed(sr);
     srunner_free(sr);

     cmd = "sed -i 's,http://check.sourceforge.net/xml/check_unittest.xslt,#style1,g' "
	     LN_TEST_RESULT_DIR"/check_output.xml";
     if (system(cmd) < 0)
	 fprintf(stderr, "system(%s) error\n", cmd);
     cmd = "sed -i '3i\\<doc>' "LN_TEST_RESULT_DIR"/check_output.xml";
     if (system(cmd) < 0)
	 fprintf(stderr, "system(%s) error\n", cmd);
     cmd = "echo '<style>' >> "LN_TEST_RESULT_DIR"/check_output.xml";
     if (system(cmd) < 0)
	 fprintf(stderr, "system(%s) error\n", cmd);
     cmd = "cat "LN_TEST_RESULT_DIR"/check_unittest.xslt >> "
	     LN_TEST_RESULT_DIR"/check_output.xml";
     if (system(cmd) < 0)
	 fprintf(stderr, "system(%s) error\n", cmd);
     cmd = "sed -i 's,<xsl:stylesheet,<xsl:stylesheet id=\"style1\",g' "
	     LN_TEST_RESULT_DIR"/check_output.xml";
     if (system(cmd) < 0)
	     fprintf(stderr, "system(%s) error\n", cmd);
     cmd = "echo '</style></doc>' >> "LN_TEST_RESULT_DIR"/check_output.xml";
     if (system(cmd) < 0)
	 fprintf(stderr, "system(%s) error\n", cmd);


     return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

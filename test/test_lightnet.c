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

#include <getopt.h>
#include <stdlib.h>
#include <check.h>
#include "test_lightnet.h"

static void print_test_usage_exit(void)
{
    const char *usage = "\
Usage: test_lightnet [OPTION...]\n\
Test lightnet.\n\
\n\
Options:\n\
  -h, --help               display this message\n\
  -f, --filter=SUITE1[|SUITE2...]\n\
                           specify test suites to be run;\n\
                           SUITE can contain '*' for glob matching;\n\
                           add '!' before the glob to take the complement;\n\
                           use '|' between SUITEs to do multiple matches;\n\
                           run all tests if omit this option\n\
";
    fputs(usage, stderr);
    exit(EXIT_SUCCESS);
}

static void exec_cmd(const char *cmd)
{
    if (system(cmd) != 0) {
        fprintf(stderr, "system(%s) error\n", cmd);
        exit(EXIT_FAILURE);
    }
}

static Suite *make_check_suite(test_record *record, const char *suite_name)
{
    Suite *suite;
    TCase *(*create_tcase)(test_record *);

    suite = suite_create(suite_name);
    for (int i = 0; i < record->suites_num; i++) {
        if (!strcmp(record->suite_names[i], suite_name)) {
            for (int j = 0; j < record->tcase_nums[i]; j++) {
                create_tcase = record->tcase_ptrs_array[i][j];
                suite_add_tcase(suite, create_tcase(record));
            }
        }
    }

    return suite;
}

static int run_tests(const char *filter)
{
    int number_failed;
    SRunner *sr;
    test_record *record, *filtered_record;

    record = test_record_create();
    add_util_record(record);
#ifdef LN_CUDA
    add_cuda_record(record);
#endif
    add_list_record(record);
    add_queue_record(record);
    add_stack_record(record);
    add_hash_record(record);
    add_graph_record(record);
    add_msg_record(record);
    add_param_record(record);
    add_tensor_record(record);
    add_op_record(record);
    add_parse_record(record);
    add_mem_record(record);
    add_pass_record(record);
    add_opimpl_record(record);
     /* end of adding suites */

    sr = srunner_create(make_master_suite());
    srunner_set_xml (sr, LN_TEST_RESULT_DIR"/check_output.xml");
    filtered_record = test_record_create_filtered(record, filter);
    for (int i = 0; i < filtered_record->suites_num; i++) {
        if (filtered_record->tcase_nums[i] < 0) {
            srunner_add_suite(
                sr, make_check_suite(record, filtered_record->suite_names[i]));
            continue;
        }
        /* TODO: not support select tcase now */
    }

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    test_record_free(filtered_record);
    test_record_free(record);

    return number_failed;
}

int main(int argc, char **argv)
{
    int optindex, opt;
    const char *filter = NULL;
    const struct option longopts[] = {
        {"help",   no_argument, NULL, 'h'},
        {"filter", required_argument, NULL, 'f'},
        {0, 0, 0, 0}
    };

    optind = 1;
    while ((opt = getopt_long_only(argc, argv, ":hf:", longopts,
                                   &optindex)) != -1) {
        switch (opt) {
        case 0:
            break;
        case 'h':
            print_test_usage_exit();
            break;
        case 'f':
            filter = optarg;
            break;
        case ':':
            fprintf(stderr, "option %s needs a value\n", argv[optind-1]);
            exit(EXIT_FAILURE);
            break;
        case '?':
            fprintf(stderr, "unknown option %s\n", argv[optind-1]);
            exit(EXIT_FAILURE);
            break;
        default:
            fprintf(stderr, "getopt_long_only() returned character code %d\n",
                    opt);
            exit(EXIT_FAILURE);
            break;
        }
    }

    int number_failed = run_tests(filter);

    exec_cmd("sed -i 's,http://check.sourceforge.net/xml/check_unittest.xslt,#style,g' "
             LN_TEST_RESULT_DIR"/check_output.xml");
    exec_cmd("sed -i '3i\\<doc>' "LN_TEST_RESULT_DIR"/check_output.xml");
    exec_cmd("echo '<style>' >> "LN_TEST_RESULT_DIR"/check_output.xml");
    exec_cmd("cat "LN_TEST_DATA_DIR"/check_unittest.xslt >> "
             LN_TEST_RESULT_DIR"/check_output.xml");
    exec_cmd("sed -i 's,<xsl:stylesheet,<xsl:stylesheet id=\"style\",g' "
             LN_TEST_RESULT_DIR"/check_output.xml");
    exec_cmd("echo '</style></doc>' >> "LN_TEST_RESULT_DIR"/check_output.xml");

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*
 * Copyright (c) 2018-2020 Zhixu Zhao
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

#include "ln_test.h"

ln_test_info _global_test_info;

static void __attribute__ ((destructor)) _clean_global_record(void)
{
    ln_test_record_finalize(&_global_test_info.record);
}


static Suite *add_check_suite(const ln_test_record *record, SRunner *sr)
{
    Suite *suite;
    TCase *(*create_tcase)(ln_test_record *, const char *const *, int);

    for (int i = 0; i < record->suites_num; i++) {
        suite = suite_create(record->suite_names[i]);
        create_tcase = record->suite_ptrs[i];
        suite_add_tcase(suite, create_tcase(NULL, record->test_names_array[i],
                                            record->test_nums[i]));
        srunner_add_suite(sr, suite);
    }

    return suite;
}

static Suite *make_master_suite(void)
{
    Suite *s;
    TCase *tc_master;

    s = suite_create("master");
    tc_master = tcase_create("master");

    suite_add_tcase(s, tc_master);

    return s;
}

static void exec_cmd(const char *cmd)
{
    if (system(cmd) != 0) {
        fprintf(stderr, "system(%s) error\n", cmd);
        exit(EXIT_FAILURE);
    }
}

void ln_test_set_verbose(int verbose)
{
    _global_test_info.verbose = verbose;
}

int ln_test_run_tests(const char *filter)
{
    int num_failed;
    SRunner *sr;
    ln_test_record *filtered_record = NULL;

    sr = srunner_create(make_master_suite());
    srunner_set_xml (sr, LN_BUILD_TEST_DIR"/result/check_output.xml");

    filtered_record = ln_test_record_create_filtered(&_global_test_info.record,
                                                     filter);
    add_check_suite(filtered_record, sr);

    srunner_run_all(sr, CK_NORMAL);
    num_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    ln_test_record_free(filtered_record);

    exec_cmd("sed -i 's,http://check.sourceforge.net/xml/check_unittest.xslt,#style,g' "
             LN_BUILD_TEST_DIR"/result/check_output.xml");
    exec_cmd("sed -i '3i\\<doc>' "LN_BUILD_TEST_DIR"/result/check_output.xml");
    exec_cmd("echo '<style>' >> "LN_BUILD_TEST_DIR"/result/check_output.xml");
    exec_cmd("cat "LN_TEST_DIR"/result/check_unittest.xslt >> "
             LN_BUILD_TEST_DIR"/result/check_output.xml");
    exec_cmd("sed -i 's,<xsl:stylesheet,<xsl:stylesheet id=\"style\",g' "
             LN_BUILD_TEST_DIR"/result/check_output.xml");
    exec_cmd("echo '</style></doc>' >> "LN_BUILD_TEST_DIR"/result/check_output.xml");

    return num_failed;
}

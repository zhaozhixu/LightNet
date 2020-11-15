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

#include <getopt.h>
#include <stdlib.h>
#include "ln_test.h"
#include "check_unittest_xslt.h"

ln_test_info _global_test_info;

static void __attribute__((destructor)) _clean_global_record(void)
{
    ln_test_record_finalize(&_global_test_info.record);
}

void ln_test_set_verbose(int verbose)
{
    _global_test_info.verbose = verbose;
}

static Suite *add_check_suite(const ln_test_record *record, SRunner *sr)
{
    Suite *suite;
    TCase *(*create_tcase)(ln_test_record *, const char *const *, int);

    for (int i = 0; i < record->suites_num; i++) {
        suite = suite_create(record->suite_names[i]);
        create_tcase = record->suite_ptrs[i];
        suite_add_tcase(suite,
                        create_tcase(NULL, record->test_names_array[i], record->test_nums[i]));
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

static void cmd_on_file(const char *cmd, const char *file)
{
    size_t size = strlen(cmd) + strlen(file) + 2;
    char *cmd_buf = ln_test_alloc(size);
    snprintf(cmd_buf, size, "%s %s", cmd, file);
    exec_cmd(cmd_buf);
    ln_test_free(cmd_buf);
}

static void process_result_file(const char *result_file)
{
    cmd_on_file("sed -i 's,http://check.sourceforge.net/xml/check_unittest.xslt,#style,g'",
                result_file);
    cmd_on_file("sed -i '3i\\<doc>'", result_file);
    cmd_on_file("echo '<style>' >>", result_file);

    FILE *fp;
    if (!(fp = fopen(result_file, "a"))) {
        fprintf(stderr, "fopen(%s) error", result_file);
        exit(EXIT_FAILURE);
    }
    if (fwrite(check_unittest_xslt, check_unittest_xslt_len, 1, fp) < 1) {
        fprintf(stderr, "fwrite(%s) error", result_file);
        exit(EXIT_FAILURE);
    }
    if (fclose(fp) != 0) {
        fprintf(stderr, "fclose(%s) error", result_file);
        exit(EXIT_FAILURE);
    }

    cmd_on_file("sed -i 's,<xsl:stylesheet,<xsl:stylesheet id=\"style\",g'", result_file);
    cmd_on_file("echo '</style></doc>' >>", result_file);
}

int ln_test_run_tests(const char *filter, const char *result_file)
{
    int num_failed;
    SRunner *sr;
    ln_test_record *filtered_record = NULL;

    sr = srunner_create(make_master_suite());
    if (result_file && strcmp(result_file, ""))
        srunner_set_xml(sr, result_file);

    filtered_record = ln_test_record_create_filtered(&_global_test_info.record, filter);
    add_check_suite(filtered_record, sr);

    srunner_run_all(sr, CK_NORMAL);
    num_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    ln_test_record_free(filtered_record);

    process_result_file(result_file);

    return num_failed;
}

static void print_test_usage_exit(const char *prog_name)
{
    const char *usage = "\
Usage: %s [OPTION...]\n\
\n\
Options:\n\
  -h, --help               display this message\n\
  -f, --filter=SUITE1[|SUITE2...]\n\
                           specify test suites to be run;\n\
                           SUITE can contain '*' for glob matching;\n\
                           add '!' before the glob to take the complement;\n\
                           use '|' between SUITEs to do multiple matches;\n\
                           run all tests if omit this option\n\
  -o, --outfile=OUTFILE    print the result file to OUTFILE [test_output.xml]\n\
";
    size_t size = strlen(usage) + strlen(prog_name) + 10;
    char *buf = ln_test_alloc(size);
    snprintf(buf, size, usage, prog_name);
    fputs(usage, stderr);
    ln_test_free(buf);
    exit(EXIT_SUCCESS);
}

int ln_test_main(int argc, char **argv)
{
    int optindex, opt, num_failed;
    const char *filter = "*";
    const char *outfile = "test_output.xml";
    const struct option longopts[] = {
        {"help",   no_argument, NULL, 'h'},
        {"filter", required_argument, NULL, 'f'},
        {"outfile", required_argument, NULL, 'o'},
        {"verbose", required_argument, NULL, 'v'},
        {0, 0, 0, 0}
    };

    optind = 1;
    while ((opt = getopt_long_only(argc, argv, ":hfo:v", longopts,
                                   &optindex)) != -1) {
        switch (opt) {
        case 0:
            break;
        case 'h':
            print_test_usage_exit(argv[0]);
            break;
        case 'f':
            filter = optarg;
            break;
        case 'o':
            outfile = optarg;
            break;
        case 'v':
            ln_test_set_verbose(1);
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

    num_failed = ln_test_run_tests(filter, outfile);

    return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

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
#include <check.h>
#include "lightnettest/ln_test.h"

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

int main(int argc, char **argv)
{
    int optindex, opt, num_failed;
    const char *filter = "*";
    const struct option longopts[] = {
        {"help",   no_argument, NULL, 'h'},
        {"filter", required_argument, NULL, 'f'},
        {"verbose", required_argument, NULL, 'v'},
        {0, 0, 0, 0}
    };

    optind = 1;
    while ((opt = getopt_long_only(argc, argv, ":hf:v", longopts,
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

    num_failed = ln_test_run_tests(filter);

    return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

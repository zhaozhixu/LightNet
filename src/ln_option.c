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

#include <getopt.h>
#include "ln_option.h"
#include "ln_lightnet.h"

static void print_version_exit(void)
{
    char version_str[20] = {0};
    const char *version = "\
lightnet %s\n\
Copyright (C) 2018, Zhao Zhixu\n\
Released under the MIT License.\n\
";

    snprintf(version_str, 20, "%d.%d.%d",
             LN_MAJOR_VERSION, LN_MINOR_VERSION, LN_MICRO_VERSION);
    fprintf(stderr, version, version_str);
    exit(EXIT_SUCCESS);
}

static void print_usage_exit(void)
{
    const char *usage = "\
Usage: lightnet [OPTION...] SOURCE\n\
Apply compilation procedures to SOURCE according to the options.\n\
If SOURCE is -, read standard input.\n\
\n\
Options:\n\
  -h, --help             display this message\n\
  -v, --version          display version information\n\
  -o, --outfile=FILE     specify output file name; if FILE is -, print to\n\
                         standard output; if FILE is !, do not print;\n\
                         (default: out.json)\n\
  -t, --target=TARGET    specify target platform (default: cpu)\n\
  -c, --compile          compile only; do not run\n\
  -r, --run              run only; do not compile; SOURCE should have been\n\
                         memory-planned\n\
  -Wwarn                 display warnings (default)\n\
  -w, -Wno-warn          do not display warnings\n\
  -Winter                display internal warnings (default)\n\
  -Wno-inter             do not display internal warnings\n\
  -debug                 display debug messages (only works with LN_DEBUG\n\
                         defined when compiling)\n\
";
    fputs(usage, stderr);
    exit(EXIT_SUCCESS);
}

ln_option ln_option_get(int argc, char **argv)
{
    int opt;
    int optindex;
    ln_option option = {
        .source = NULL,
        .outfile = NULL,
        .target = NULL,
        .datafile = NULL,
        .compile = 1,
        .run = 1,
        .Winter = 1,
        .Wwarn = 1,
        .debug = 0,
    };
    const struct option longopts[] = {
        {"help",      no_argument, NULL, 'h'},
        {"version",   no_argument, NULL, 'v'},
        {"outfile",   required_argument, NULL, 'o'},
        {"target",    required_argument, NULL, 't'},
        {"datafile",  required_argument, NULL, 'f'},
        {"compile",   no_argument, NULL, 'c'},
        {"run",       no_argument, NULL, 'r'},
        {"Winter",    no_argument, &option.Winter, 1},
        {"Wno-inter", no_argument, &option.Winter, 0},
        {"Wwarn",     no_argument, &option.Wwarn, 1},
        {"Wno-warn",  no_argument, &option.Wwarn, 0}, /* w */
        {"debug",     no_argument, &option.debug, 1}, /* d */
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long_only(argc, argv, ":hvo:t:f:crwd",
                                   longopts, &optindex)) != -1) {
        switch (opt) {
        case 0:
            break;
        case 'h':
            print_usage_exit();
            break;
        case 'v':
            print_version_exit();
            break;
        case 'o':
            option.outfile = optarg;
            break;
        case 't':
            option.target = optarg;
            break;
        case 'f':
            option.datafile = optarg;
            break;
        case 'c':
            if (option.compile == 0 && option.run == 1) {
                option.compile = 1;
                break;
            }
            option.compile = 1;
            option.run = 0;
            break;
        case 'r':
            if (option.compile == 1 && option.run == 0) {
                option.run = 1;
                break;
            }
            option.compile = 0;
            option.run = 1;
            break;
        case 'w':
            option.Wwarn = 0;
            break;
        case 'd':
            option.debug = 1;
            break;
        case ':':
            ln_msg_error("option %s needs a value", argv[optind-1]);
            break;
        case '?':
            ln_msg_error("unknown option %s", argv[optind-1]);
            break;
        default:
            ln_msg_inter_error("getopt() returned character code %d", opt);
            break;
        }
    }
    if (optind >= argc)
        ln_msg_error("no input file");
    else
        option.source = argv[optind++];

    if (!option.outfile)
        option.outfile = "out.json";
    if (!option.target)
        option.target = "cpu";

    return option;
}

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

#include <stdlib.h>
#include <getopt.h>
#include "ln_option.h"
#include "ln_util.h"
#include "lightnet.h"

LN_EXPORT void ln_option_sprint_version(char *version_str)
{
    snprintf(version_str, 20, "%d.%d.%d",
             LN_MAJOR_VERSION, LN_MINOR_VERSION, LN_MICRO_VERSION);
}

static void print_version_exit(void)
{
    char version_str[20] = {0};
    const char *version = "\
lightnet %s\n\
Copyright (c) 2018-2020, Zhixu Zhao\n\
Released under the MIT License.\n\
";

    ln_option_sprint_version(version_str);
    fprintf(stderr, version, version_str);
    exit(EXIT_SUCCESS);
}

static void print_usage_exit(void)
{
    const char *usage = "\
Usage: lightnet [OPTION...] SOURCE\n\
Compile a neural network model SOURCE and/or run the compiled model.\n\
If SOURCE is -, read standard input.\n\
\n\
Options:\n\
  -h, --help             display this message\n\
  -v, --version          display version information\n\
  -o, --outfile=FILE     specify output file name; if FILE is -, print to\n\
                         standard output; if FILE is !, do not print;\n\
                         (default: out.json)\n\
  -t, --target=TARGET    specify target platform (default: cpu)\n\
  -f, --datafile=FILE    specify tensor data file\n\
  -c, --compile          compile only; do not run\n\
  -r, --run              run only; do not compile; SOURCE should have been\n\
                         memory-planned\n\
  -d, --debug            display debug messages (only works with LN_DEBUG\n\
                         defined when compiling)\n\
  -Wwarn                 display warnings (default)\n\
  -w, -Wno-warn          do not display warnings\n\
  -Winter                display internal warnings (default)\n\
  -Wno-inter             do not display internal warnings\n\
";
    fputs(usage, stderr);
    exit(EXIT_SUCCESS);
}

LN_EXPORT ln_option *ln_option_create(int argc, char **argv)
{
    int opt;
    int optindex;
    ln_option *option = ln_alloc(sizeof(ln_option));

    option->argv = ln_strarraydup(argv, argc);
    option->argc = argc;
    option->source = NULL;
    option->outfile = NULL;
    option->target = NULL;
    option->datafile = NULL;
    option->compile = 1;
    option->run = 1;
    option->Winter = 1;
    option->Wwarn = 1;
    option->debug = 0;

    const struct option longopts[] = {
        {"help",      no_argument, NULL, 'h'},
        {"version",   no_argument, NULL, 'v'},
        {"outfile",   required_argument, NULL, 'o'},
        {"target",    required_argument, NULL, 't'},
        {"datafile",  required_argument, NULL, 'f'},
        {"compile",   no_argument, NULL, 'c'},
        {"run",       no_argument, NULL, 'r'},
        {"Winter",    no_argument, &option->Winter, 1},
        {"Wno-inter", no_argument, &option->Winter, 0},
        {"Wwarn",     no_argument, &option->Wwarn, 1},
        {"Wno-warn",  no_argument, &option->Wwarn, 0}, /* w */
        {"debug",     no_argument, &option->debug, 1}, /* d */
        {0, 0, 0, 0}
    };

    optind = 1;
    while ((opt = getopt_long_only(option->argc, option->argv, ":hvo:t:f:crwd",
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
            option->outfile = optarg;
            break;
        case 't':
            option->target = optarg;
            break;
        case 'f':
            option->datafile = optarg;
            break;
        case 'c':
            if (option->compile == 0 && option->run == 1) {
                option->compile = 1;
                break;
            }
            option->compile = 1;
            option->run = 0;
            break;
        case 'r':
            if (option->compile == 1 && option->run == 0) {
                option->run = 1;
                break;
            }
            option->compile = 0;
            option->run = 1;
            break;
        case 'w':
            option->Wwarn = 0;
            break;
        case 'd':
            option->debug = 1;
            break;
        case ':':
            ln_msg_error("option %s needs a value", option->argv[optind-1]);
            break;
        case '?':
            ln_msg_error("unknown option %s", option->argv[optind-1]);
            break;
        default:
            ln_msg_inter_error("getopt_long_only() returned character code %d",
                               opt);
            break;
        }
    }
    if (optind >= option->argc)
        ln_msg_error("no input file");
    else
        option->source = option->argv[optind++];

    if (!option->outfile)
        option->outfile = "out.json";
    if (!option->target)
        option->target = "cpu";

    return option;
}

LN_EXPORT void ln_option_free(ln_option *option)
{
    ln_strarray_free(option->argv, option->argc);
    ln_free(option);
}

LN_EXPORT const char *ln_option_get_source(ln_option *option)
{
    return option->source;
}

LN_EXPORT const char *ln_option_get_outfile(ln_option *option)
{
    return option->outfile;
}

LN_EXPORT const char *ln_option_get_target(ln_option *option)
{
    return option->target;
}

LN_EXPORT const char *ln_option_get_datafile(ln_option *option)
{
    return option->datafile;
}

LN_EXPORT int ln_option_get_compile(ln_option *option)
{
    return option->compile;
}

LN_EXPORT int ln_option_get_run(ln_option *option)
{
    return option->run;
}

LN_EXPORT int ln_option_get_Winter(ln_option *option)
{
    return option->Winter;
}

LN_EXPORT int ln_option_get_Wwarn(ln_option *option)
{
    return option->Wwarn;
}

LN_EXPORT int ln_option_get_debug(ln_option *option)
{
    return option->debug;
}

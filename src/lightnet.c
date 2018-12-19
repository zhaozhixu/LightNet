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
#include "lightnet.h"

static void get_options(int argc, char **argv);

static ln_option option = {
    .source = NULL,
    .outfile = NULL,
    .target = NULL,
    .run = 0,
    .Winter = 1,
    .Wwarn = 1,
    .debug = 0,
};

int main(int argc, char **argv)
{
    ln_arch *arch;
    ln_context *ctx;

    get_options(argc, argv);
    ln_msg_init(&option);
    ln_arch_init();
    ln_name_init();
    ctx = ln_context_create();

    ln_json_parse_file(option.source, ctx);
    ln_context_init_ops(ctx);

    arch = ln_hash_find(LN_ARCH.arch_table, option.target);
    ln_pass_expander(ctx, arch->ep_funcs);
    ln_pass_combiner(ctx, 3, arch->cb_funcs);
    ln_json_print_file(option.outfile, ctx);

    if (option.run)
        ln_context_run(ctx);

    ln_arch_cleanup();
    ln_name_cleanup();
    ln_context_free(ctx);

    return 0;
}

static void print_usage_exit(void)
{
    const char *usage = "\
Usage: lightnet [OPTION...] SOURCE\n\
Apply optimization procedures to SOURCE according to the options.\n\
When SOURCE is -, read standard input.\n\
\n\
Options:\n\
  -h, --help             display this message\n\
  -v, --version          display version information\n\
  -o, --outfile=FILE     specify output file name; when FILE is -, print to\n\
                         standard output (default: out.json)\n\
  -t, --target=TARGET    specify target platform (default: cpu)\n\
  -r, --run              compile and run the model\n\
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

static void get_options(int argc, char **argv)
{
    int opt;
    int optindex;
    const struct option longopts[] = {
        {"help",      no_argument, NULL, 'h'},
        {"version",   no_argument, NULL, 'v'},
        {"outfile",   required_argument, NULL, 'o'},
        {"target",    required_argument, NULL, 't'},
        {"run",       no_argument, &option.run, 1}, /* r */
        {"Winter",    no_argument, &option.Winter, 1},
        {"Wno-inter", no_argument, &option.Winter, 0},
        {"Wwarn",     no_argument, &option.Wwarn, 1},
        {"Wno-warn",  no_argument, &option.Wwarn, 0}, /* w */
        {"debug",     no_argument, &option.debug, 1}, /* d */
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long_only(argc, argv, ":hvo:t:rwd",
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
        case 'r':
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
}

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

static const struct option longopts[] = {
    {"run", 0, NULL, 'r'},
    {"target", 1, NULL, 't'},
    {"outfile", 1, NULL, 'o'},
    {"help", 0, NULL, 'h'},
    {0, 0, 0, 0}
};

int main(int argc, char **argv)
{
    char *json_file;
    char *target;
    ln_arch *arch;
    ln_context *ctx;

    ln_arch_init();
    ln_name_init();
    ctx = ln_context_create();

    json_file = argv[1];
    target = argv[2];
    ln_json_parse_file(json_file, ctx);
    ln_context_init_ops(ctx);

    arch = ln_hash_find(LN_ARCH.arch_table, target);
    ln_pass_expander(ctx, arch->ep_funcs);
    /* ln_json_fprint(stdout, ctx); */
    ln_pass_combiner(ctx, 3, arch->cb_funcs);
    /* ln_pass_mem_plan(ctx); */
    ln_json_fprint(stdout, ctx);

    /* ln_context_alloc_mem(ctx); */
    /* ln_context_static_run(ctx); */
    /* ln_context_run(ctx); */
    /* ln_context_cleanup_ops(ctx); */
    /* ln_context_dealloc_mem(ctx); */
    ln_context_run(ctx);

    ln_arch_cleanup();
    ln_name_cleanup();
    ln_context_free(ctx);
    ln_cuda_device_reset();
}

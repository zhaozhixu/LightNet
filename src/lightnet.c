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

#include "ln_lightnet.h"

int main(int argc, char **argv)
{
    ln_context *ctx;
    ln_option option;

    option = ln_option_get(argc, argv);
    ln_msg_init(&option);
    ln_arch_init();
    ln_name_init();
    ctx = ln_context_create();

    ln_context_init(ctx, option.source);

    if (option.compile)
        ln_context_compile(ctx, option.target);

    if (!ln_streq(option.outfile, "!"))
        ln_context_print(ctx, option.outfile);

    if (option.run) {
        ln_context_load(ctx);
        ln_context_run(ctx);
        ln_context_unload(ctx);
    }

    ln_context_cleanup(ctx);

    ln_context_free(ctx);
    ln_arch_cleanup();
    ln_name_cleanup();

    return 0;
}

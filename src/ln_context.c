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

#include "ln_context.h"
#include "ln_arch.h"

ln_context ln_global_context;

void ln_context_init(void)
{
    ln_list *init_ops;

    LN_CTX.arch_table = ln_arch_table_create();

    init_ops = ln_arch_create_oplist();
    LN_CTX.op_init_table = ln_op_init_table_create(init_ops);
    ln_op_list_free(init_ops);

    LN_CTX.tensor_table = ln_tensor_table_create();
}

void ln_context_cleanup(void)
{
    ln_arch_table_free(LN_CTX.arch_table);
    ln_op_init_table_free(LN_CTX.op_init_table);
    ln_tensor_table_free(LN_CTX.tensor_table);
}

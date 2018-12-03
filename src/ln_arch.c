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

#include "ln_arch.h"

extern ln_arch ln_arch_cpu;

#ifdef LN_CUDA
extern ln_arch ln_arch_cuda;
#endif

#ifdef LN_TENSORRT
extern ln_arch ln_arch_tensorrt;
#endif

static ln_arch *init_archs[] = {
    &ln_arch_cpu,
#ifdef LN_CUDA
    &ln_arch_cuda,
#endif
#ifdef LN_TENSORRT
    &ln_arch_tensorrt,
#endif
    NULL
};

ln_init_tables ln_global_init_tables;

static ln_hash *arch_table_create(ln_arch *archs[])
{
    ln_hash *arch_table;
    int i;
    int ret;

    arch_table = ln_hash_create(ln_str_hash, ln_str_cmp, NULL, NULL);
    for (i = 0; archs[i]; i++) {
        ret = ln_hash_insert(arch_table, archs[i]->arch_name, archs[i]);
        assert(ret && "duplicated arch_name");
    }

    return arch_table;
}

static void arch_table_free(ln_hash *arch_table)
{
    ln_hash_free(arch_table);
}

static ln_hash *init_op_table_create(ln_arch *archs[])
{
    ln_hash *init_op_table;
    ln_op *op;
    int i, j;

    init_op_table = ln_hash_create(ln_str_hash, ln_str_cmp, NULL, NULL);
    for (i = 0; archs[i]; i++) {
        for (j = 0; (op = archs[i]->reg_ops[j]); j++)
            ln_hash_insert(init_op_table, op->op_arg->optype, op);
    }

    return init_op_table;
}

static void init_op_table_free(ln_hash *op_init_table)
{
    ln_hash_free(op_init_table);
}

void ln_arch_init(void)
{
    LN_INIT.init_arch_table = arch_table_create(init_archs);
    LN_INIT.init_op_table = init_op_table_create(init_archs);
}

void ln_arch_cleanup(void)
{
    arch_table_free(LN_INIT.init_arch_table);
    init_op_table_free(LN_INIT.init_op_table);
}

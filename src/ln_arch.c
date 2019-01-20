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

extern ln_arch ln_arch_none;
extern ln_arch ln_arch_cpu;

#ifdef LN_CUDA
extern ln_arch ln_arch_cuda;
#endif

#ifdef LN_CUDNN
extern ln_arch ln_arch_cudnn;
#endif

#ifdef LN_TENSORRT
extern ln_arch ln_arch_tensorrt;
#endif

static ln_arch *archs[] = {
    &ln_arch_none,
    &ln_arch_cpu,
#ifdef LN_CUDA
    &ln_arch_cuda,
#endif
#ifdef LN_CUDNN
    &ln_arch_cudnn,
#endif
#ifdef LN_TENSORRT
    &ln_arch_tensorrt,
#endif
    NULL
};

ln_arch_info ln_global_arch_info;

void ln_arch_init(void)
{
    ln_op *op;
    int ret;
    int i, j;

    LN_ARCH.arch_table = ln_hash_create(ln_str_hash, ln_str_cmp, NULL, NULL);
    LN_ARCH.op_proto_table = ln_hash_create(ln_str_hash, ln_str_cmp, NULL, NULL);

    for (i = 0; archs[i]; i++) {
        if (archs[i]->init_func)
            archs[i]->init_func(&archs[i]->context);
        ret = ln_hash_insert(LN_ARCH.arch_table, archs[i]->arch_name, archs[i]);
        if (!ret)
            ln_msg_inter_error("duplicated arch name \"%s\"",
                               archs[i]->arch_name);

        for (j = 0; (op = archs[i]->reg_ops[j]); j++) {
            ret = ln_hash_insert(LN_ARCH.op_proto_table, op->op_arg->optype, op);
            if (!ret)
                ln_msg_inter_error("duplicated optype \"%s\"",
                                   op->op_arg->optype);
        }
    }
}

void ln_arch_cleanup(void)
{
    int i;

    for (i = 0; archs[i]; i++) {
        if (archs[i]->cleanup_func)
            archs[i]->cleanup_func(&archs[i]->context);
    }

    ln_hash_free(LN_ARCH.arch_table);
    ln_hash_free(LN_ARCH.op_proto_table);
}

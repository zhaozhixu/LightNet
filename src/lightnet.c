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
    {"", 1, NULL, 'e'},
    {"video", 1, NULL, 'v'},
    {"bbox-dir", 1, NULL, 'b'},
    {"x-shift", 1, NULL, 'x'},
    {"y-shift", 1, NULL, 'y'},
    {"help", 0, NULL, 'h'},
    {0, 0, 0, 0}
};

int main(int argc, char **argv)
{
    char *json_file;
    char *json_str;
    char *target;
    ln_hash *arch_table;
    ln_hash *tensor_table;
    ln_hash *reg_op_table;
    ln_hash *op_table;
    ln_list *ops;
    ln_list *reg_ops;
    ln_arch *arch;
    ln_error *error = NULL;

    arch_table = ln_arch_table_create();
    tensor_table = ln_tensor_table_create();
    reg_ops = ln_arch_create_reg_ops();
    reg_op_table = ln_op_table_create(reg_ops, "optype");

    json_file = argv[1];
    target = argv[2];
    json_str = ln_read_text(json_file);
    ops = ln_json_parse(json_str, reg_ops, tensor_table);
    ln_op_list_do_pre_run(ops, &error);
    ln_error_handle(&error);
    arch = ln_hash_find(arch_table, target);
    ops = ln_pass_peephole(ops, 3, arch->ph_funcs, arch->post_ph);
    /* ln_json_fprint(stdout, ops); */

    ln_arch_table_free(arch_table);
    ln_tensor_table_free(tensor_table);
    ln_op_list_free(reg_ops);
    ln_hash_free(reg_op_table);
    ln_free(json_str);
    ln_op_list_free_lists_too(ops);
}

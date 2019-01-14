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

#include <ctype.h>
#include "ln_op.h"
#include "ln_name.h"

static ln_op_arg *ln_op_arg_create(const char *name, ln_list *tensors_in,
                                   ln_list *tensors_out, ln_list *params,
                                   ln_hash *tensor_table,
                                   const ln_op_arg *proto_arg)
{
    ln_op_arg *op_arg;

    op_arg = ln_alloc(sizeof(ln_op_arg));
    op_arg->name = ln_strdup(name);
    op_arg->optype = ln_strdup(proto_arg->optype);
    op_arg->arch = ln_strdup(proto_arg->arch),
    op_arg->tensors_in = tensors_in;
    op_arg->tensors_out = tensors_out;
    op_arg->params = params;
    op_arg->priv = NULL;
    op_arg->tensor_table = tensor_table;
    op_arg->in_arg_names = proto_arg->in_arg_names;
    op_arg->out_arg_names = proto_arg->out_arg_names;
    op_arg->param_arg_names = proto_arg->param_arg_names;

    return op_arg;
}

static void ln_op_arg_free(ln_op_arg *op_arg)
{
    ln_free(op_arg->name);
    ln_free(op_arg->optype);
    ln_free(op_arg->arch);
    ln_free(op_arg);
}

ln_op *ln_op_create_from_proto(const ln_op *op_proto, const char *name,
                               ln_list *tensors_in, ln_list *tensors_out,
                               ln_list *params, ln_hash *tensor_table)
{
    ln_op *op;

    op = ln_alloc(sizeof(ln_op));
    op->op_arg = ln_op_arg_create(name, tensors_in, tensors_out, params,
                                  tensor_table, op_proto->op_arg);
    op->pre_run = op_proto->pre_run;
    op->static_run = op_proto->static_run;
    op->run = op_proto->run;
    op->post_run = op_proto->post_run;

    return op;
}

void ln_op_free(ln_op *op)
{
    if (!op)
        return;
    ln_op_arg_free(op->op_arg);
    ln_free(op);
}

ln_op *ln_op_create_with_names(const ln_op *op_proto, ln_hash *tensor_table)
{
    ln_list *tensors_in = NULL;
    ln_list *tensors_out = NULL;
    ln_list *params = NULL;     /* TODO: not create it yet */
    const char *arg_name;
    char opname[LN_MAX_NAME_LEN];
    char tensor_name[LN_MAX_NAME_LEN];
    int i;

    strncpy(opname, ln_unique_name(op_proto->op_arg->optype), LN_MAX_NAME_LEN);
    for (i = 0; (arg_name = op_proto->op_arg->in_arg_names[i]); i++) {
        if (strlen(opname) + strlen(arg_name) + 2 <= LN_MAX_NAME_LEN)
            snprintf(tensor_name, LN_MAX_NAME_LEN, "%s_%s", opname, arg_name);
        else
            strncpy(tensor_name, ln_unique_name(arg_name), LN_MAX_NAME_LEN);
        tensors_in = ln_tensor_list_append(tensors_in, arg_name, tensor_name);
    }
    for (i = 0; (arg_name = op_proto->op_arg->out_arg_names[i]); i++) {
        if (strlen(opname) + strlen(arg_name) + 2 <= LN_MAX_NAME_LEN)
            snprintf(tensor_name, LN_MAX_NAME_LEN, "%s_%s", opname, arg_name);
        else
            strncpy(tensor_name, ln_unique_name(arg_name), LN_MAX_NAME_LEN);
        tensors_out = ln_tensor_list_append(tensors_out, arg_name, tensor_name);
    }

    return ln_op_create_from_proto(op_proto, opname, tensors_in,
                                   tensors_out, params, tensor_table);
}

ln_op *ln_op_create_with_opname(const ln_op *op_proto, ln_hash *tensor_table)
{
    ln_list *tensors_in = NULL;
    ln_list *tensors_out = NULL;
    ln_list *params = NULL;     /* TODO: not create it yet */
    const char *arg_name;
    char opname[LN_MAX_NAME_LEN];
    int i;

    strncpy(opname, ln_unique_name(op_proto->op_arg->optype), LN_MAX_NAME_LEN);
    for (i = 0; (arg_name = op_proto->op_arg->in_arg_names[i]); i++)
        ln_tensor_list_append(tensors_in, arg_name, NULL);

    for (i = 0; (arg_name = op_proto->op_arg->out_arg_names[i]); i++)
        ln_tensor_list_append(tensors_out, arg_name, NULL);

    return ln_op_create_from_proto(op_proto, opname, tensors_in,
                                   tensors_out, params, tensor_table);
}

ln_op *ln_op_copy(const ln_op *op)
{
    return ln_op_create_from_proto(op, op->op_arg->name,
                                   ln_tensor_list_copy(op->op_arg->tensors_in),
                                   ln_tensor_list_copy(op->op_arg->tensors_out),
                                   ln_param_list_copy(op->op_arg->params),
                                   op->op_arg->tensor_table);
}

ln_op *ln_op_copy_to_optype(ln_hash *op_proto_table, const ln_op *op,
                            const char *new_optype)
{
    ln_op *new_op_proto;
    ln_op *new_op;

    new_op_proto = ln_hash_find(op_proto_table, new_optype);
    if (!new_op_proto)
        ln_msg_inter_error("optype %s not found", new_optype);
    new_op = ln_op_create_from_proto(new_op_proto, op->op_arg->name,
                                     ln_tensor_list_copy(op->op_arg->tensors_in),
                                     ln_tensor_list_copy(op->op_arg->tensors_out),
                                     ln_param_list_copy(op->op_arg->params),
                                     op->op_arg->tensor_table);
    return new_op;
}

ln_tensor_entry *ln_op_find_tensor_entry(const ln_op *op, const char *arg_name)
{
    char *tname;

    tname = ln_tensor_list_find_name(op->op_arg->tensors_in, arg_name);
    if (!tname)
        tname = ln_tensor_list_find_name(op->op_arg->tensors_out, arg_name);
    if (!tname)
        return NULL;

    return ln_tensor_table_find(op->op_arg->tensor_table, tname);
}

ln_tensor_list_entry *ln_op_find_tensor_list_entry(const ln_op *op,
                                                   const char *arg_name)
{
    ln_tensor_list_entry *tle;

    tle = ln_tensor_list_find_by_arg_name(op->op_arg->tensors_in, arg_name);
    if (!tle)
        tle = ln_tensor_list_find_by_arg_name(op->op_arg->tensors_out, arg_name);
    return tle;
}

void ln_op_free_lists_too(ln_op *op)
{
    if (!op)
        return;
    ln_tensor_list_free(op->op_arg->tensors_in);
    ln_tensor_list_free(op->op_arg->tensors_out);
    ln_param_list_free(op->op_arg->params);
    ln_op_arg_free(op->op_arg);
    ln_free(op);
}

ln_list *ln_op_list_create_from_array(ln_op **op_array)
{
    ln_list *ops = NULL;
    int i;

    for (i = 0; op_array[i]; i++)
        ops = ln_list_append(ops, op_array[i]);

    return ops;
}

void ln_op_list_free(ln_list *op_list)
{
    ln_list_free(op_list);
}

static void op_free_lists_too_wrapper(void *p)
{
    ln_op *op = p;

    ln_op_free_lists_too(op);
}

void ln_op_list_free_lists_too(ln_list *ops)
{
    ln_list_free_deep(ops, op_free_lists_too_wrapper);
}

static int cmp_by_optype(const void *data1, const void *data2)
{
    const ln_op *op1 = data1;
    const ln_op *op2 = data2;

    return strcmp(op1->op_arg->optype, op2->op_arg->optype);
}

ln_op *ln_op_list_find_by_optype(ln_list *ops, const char *optype)
{
    ln_op op_hint;
    ln_op_arg op_arg_hint;
    ln_op *result_op = NULL;

    op_arg_hint.optype = (char *)optype;
    op_hint.op_arg = &op_arg_hint;
    result_op = ln_list_find_custom(ops, &op_hint, cmp_by_optype);

    return result_op;
}

ln_op *ln_op_array_find_by_optype(ln_op *ops[], const char *optype)
{
    ln_op op_hint;
    ln_op_arg op_arg_hint;
    ln_op *result_op = NULL;
    int i;

    op_arg_hint.optype = (char *)optype;
    op_hint.op_arg = &op_arg_hint;
    for (i = 0; ops[i]; i++) {
        if (!cmp_by_optype(ops[i], &op_hint)) {
            result_op = ops[i];
            break;
        }
    }

    return result_op;
}

static int cmp_by_name(const void *data1, const void *data2)
{
    const ln_op *op1 = data1;
    const ln_op *op2 = data2;

    return strcmp(op1->op_arg->name, op2->op_arg->name);
}

ln_op *ln_op_list_find_by_name(ln_list *ops, const char *name)
{
    ln_op op_hint;
    ln_op_arg op_arg_hint;
    ln_op *result_op = NULL;

    op_arg_hint.name = (char *)name;
    op_hint.op_arg = &op_arg_hint;
    result_op = ln_list_find_custom(ops, &op_hint, cmp_by_name);

    return result_op;
}

void ln_op_list_do_pre_run(ln_list *ops)
{
    ln_op *op;
    ln_list *l;

    for (l = ops; l; l = l->next) {
        op = l->data;
        op->pre_run(op->op_arg);
    }
}

void ln_op_list_do_static_run(ln_list *ops)
{
    ln_op *op;
    ln_list *l;

    for (l = ops; l; l = l->next) {
        op = l->data;
        if (!op->static_run)
            continue;
        op->static_run(op->op_arg);
    }
}

void ln_op_list_do_run(ln_list *ops)
{
    ln_op *op;
    ln_list *l;

    for (l = ops; l; l = l->next) {
        op = l->data;
        if (!op->run)
            continue;
        op->run(op->op_arg);
    }
}

void ln_op_list_do_post_run(ln_list *ops)
{
    ln_op *op;
    ln_list *l;

    for (l = ops; l; l = l->next) {
        op = l->data;
        op->post_run(op->op_arg);
    }
}

char *ln_op_list_new_opname(const ln_list *ops, const char *prefix)
{
    ln_op *op;
    char *buf;
    size_t prefix_len = strlen(prefix);
    size_t buf_len = prefix_len + LN_MAX_NAME_SUBFIX;
    int max_idx = 0;
    int idx;

    buf = ln_alloc(sizeof(char)*buf_len);
    LN_LIST_FOREACH(op, ops) {
        if (!ln_streqn(op->op_arg->name, prefix, prefix_len))
            continue;
        assert(isdigit(op->op_arg->name[prefix_len]) &&
               "subfixed with no digit");
        idx = atoi(&op->op_arg->name[prefix_len]);
        max_idx = max_idx < idx ? idx : max_idx;
    }
    snprintf(buf, buf_len, "%s%d", prefix, max_idx);
    return buf;
}

ln_hash *ln_op_table_create(void)
{
    return ln_hash_create(ln_str_hash, ln_str_cmp, NULL,
                          op_free_lists_too_wrapper);
}

int ln_op_table_insert(ln_hash *table, ln_op *op)
{
    return ln_hash_insert(table, op->op_arg->name, op);
}

int ln_op_table_remove(ln_hash *table, const char *name)
{
    return ln_hash_remove(table, (char *)name);
}

ln_op *ln_op_table_find(ln_hash *table, const char *name)
{
    return ln_hash_find(table, (char *)name);
}

void ln_op_table_free(ln_hash *table)
{
    ln_hash_free(table);
}

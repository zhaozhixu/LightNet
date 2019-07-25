/*
 * Copyright (c) 2018-2019 Zhao Zhixu
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

#include "ln_arch.h"
#include "ln_name.h"
#include "ln_tensorrt.h"

extern ln_op ln_opimpl_tensorrt;
/* end of declare tensorrt ops */

static ln_op *ops_tensorrt[] = {
    &ln_opimpl_tensorrt,
/* end of init tensorrt ops */
    NULL
};

extern ln_list *ln_expander_expander_tensorrt(const ln_op *op, const ln_dfg *dfg, int *match);
/* end of declare tensorrt expanders */

ln_expander_func ep_funcs_tensorrt[] = {
    ln_expander_expander_tensorrt,
/* end of tensorrt expanders */
    NULL
};

static ln_list *cb_func_tensorrt(const ln_list *win_ops, size_t win_size,
                                 const ln_dfg *dfg, int *match);
/* end of declare tensorrt combiners */

ln_combiner_func cb_funcs_tensorrt[] = {
    cb_func_tensorrt,
/* end of tensorrt combiners */
    NULL
};

/* end of declare tensorrt subgraphers */

ln_subgraph_func sg_funcs_tensorrt[] = {
/* end of tensorrt subgraphers */
    NULL
};

/* end of declare tensorrt schedulers */

ln_schedule_func sd_funcs_tensorrt[] = {
/* end of tensorrt schedulers */
    NULL
};

extern void ln_expander_init_expander_tensorrt(void **priv_p);
/* end of declare tensorrt init funcs */

static void init_tensorrt(void **priv_p)
{
    ln_expander_init_expander_tensorrt(priv_p);
/* end of exec tensorrt init funcs */
}

extern void ln_expander_cleanup_expander_tensorrt(void **priv_p);
/* end of declare tensorrt cleanup funcs */

static void cleanup_tensorrt(void **priv_p)
{
    ln_expander_cleanup_expander_tensorrt(priv_p);
/* end of exec tensorrt cleanup funcs */
}

/* TODO: use char[] to replace ln_strcat() etc. */
static char *create_arg_name_in_tensors(ln_list *tensors, const char *prefix)
{
    ln_tensor_list_entry *tle;
    int max_idx = -1;
    char *buf;
    size_t prefix_len = strlen(prefix);
    size_t buf_len = prefix_len + LN_MAX_NAME_SUBFIX;

    buf = ln_alloc(sizeof(char)*buf_len);
    LN_LIST_FOREACH(tle, tensors) {
        if (!ln_streqn(tle->arg_name, prefix, prefix_len) ||
            ln_next_token(tle->arg_name, '_'))
            continue;
        assert(isdigit(tle->arg_name[prefix_len]) && "subfixed with no digit");
        int idx = atoi(&tle->arg_name[prefix_len]);
        max_idx = max_idx < idx ? idx : max_idx;
    }
    snprintf(buf, buf_len, "%s%d", prefix, max_idx+1);
    return buf;
}

static char *create_arg_name_in_params(ln_list *params, const char *prefix)
{
    ln_param_entry *pe;
    int max_idx = -1;
    char *buf;
    size_t prefix_len = strlen(prefix);
    size_t buf_len = prefix_len + LN_MAX_NAME_SUBFIX + 1;

    buf = ln_alloc(sizeof(char)*buf_len);
    LN_LIST_FOREACH(pe, params) {
        if (!ln_streqn(pe->arg_name, prefix, prefix_len) ||
            ln_next_token(pe->arg_name, '_'))
            continue;
        assert(isdigit(pe->arg_name[prefix_len]) && "subfixed with no digit");
        int idx = atoi(&pe->arg_name[prefix_len]);
        max_idx = max_idx < idx ? idx : max_idx;
    }
    snprintf(buf, buf_len, "%s%d", prefix, max_idx+1);
    return buf;
}

static int exists_in_tensors(ln_list *tensors, const char *name)
{
    ln_tensor_list_entry *tle;

    LN_LIST_FOREACH(tle, tensors) {
        if (ln_streq(tle->name, name))
            return 1;
    }
    return 0;
}

static void add_trt_src(ln_op_arg *trt_arg, ln_op_arg *arg, char *tensor_name)
{
    char *tensor_arg_name;

    if (!exists_in_tensors(trt_arg->tensors_out, tensor_name) &&
        !exists_in_tensors(trt_arg->tensors_in, tensor_name)) {
        tensor_arg_name = create_arg_name_in_tensors(trt_arg->tensors_in,
                                                     "src");
        trt_arg->tensors_in = ln_tensor_list_append(trt_arg->tensors_in,
                                                    tensor_arg_name,
                                                    tensor_name);
        ln_free(tensor_arg_name);
    }
}

static void add_trt_weight(ln_op_arg *trt_arg, ln_op_arg *arg, char *tensor_name)
{
    char *tensor_arg_name;

    if (!exists_in_tensors(trt_arg->tensors_in, tensor_name)) {
        tensor_arg_name = create_arg_name_in_tensors(trt_arg->tensors_in,
                                                     "weight");
        trt_arg->tensors_in = ln_tensor_list_append(trt_arg->tensors_in,
                                                    tensor_arg_name,
                                                    tensor_name);
        ln_free(tensor_arg_name);
    }
}

static void add_trt_dst(ln_op_arg *trt_arg, ln_op_arg *arg, char *tensor_name)
{
    char *param_arg_name;
    char *tensor_arg_name;
    ln_tensor_entry *te;

    if (exists_in_tensors(trt_arg->tensors_out, tensor_name))
        return;

    tensor_arg_name = create_arg_name_in_tensors(trt_arg->tensors_out, "dst");
    trt_arg->tensors_out = ln_tensor_list_append(trt_arg->tensors_out,
                                                 tensor_arg_name,
                                                 tensor_name);

    param_arg_name = ln_strcat_delim_alloc(tensor_arg_name, "shape", '_');
    te = ln_tensor_table_find(arg->tensor_table, tensor_name);
    trt_arg->params = ln_param_list_append_array_int(trt_arg->params,
                                                     param_arg_name,
                                                     te->tensor->ndim,
                                                     te->tensor->dims);
    ln_free(param_arg_name);

    param_arg_name = ln_strcat_delim_alloc(tensor_arg_name, "dtype", '_');
    trt_arg->params = ln_param_list_append_string(trt_arg->params,
                                                  param_arg_name,
                                                  tl_dtype_name(te->tensor->dtype));
    ln_free(param_arg_name);
    ln_free(tensor_arg_name);
}

static int digits_len(int n)
{
    int count = 0;

    if (n == 0)
        return 1;

    while (n) {
        n /= 10;
        count++;
    }
    return count;
}

static char *new_arg_name(const char *old_name, int base_idx)
{
    char *new_name;
    char *next_token;
    int old_idx, new_idx;
    size_t new_name_len;

    old_idx = atoi(&old_name[2]); /* 2 for length of "op" */
    new_idx = old_idx + base_idx;
    new_name_len = strlen(old_name) - digits_len(old_idx) + digits_len(new_idx);
    new_name = ln_alloc(sizeof(char) * (new_name_len + 1));

    if ((next_token = ln_next_token(old_name, '_')))
        snprintf(new_name, new_name_len + 1, "op%d_%s", new_idx, next_token);
    else
        snprintf(new_name, new_name_len + 1, "op%d", new_idx);

    return new_name;
}

static int pe_cmp(const void *p1, const void *p2)
{
    const ln_param_entry *pe1 = p1;
    const ln_param_entry *pe2 = p2;

    return strcmp(pe1->arg_name, pe2->arg_name);
}

static void pe_free(void *p)
{
    ln_param_entry_free(p);
}

static int have_successor_except(const ln_op *trt_op, const char *tname,
                                 const ln_op *op, const ln_dfg *dfg)
{
    ln_list *suc_ens;
    ln_graph_edge_node *en;
    ln_op *suc_op;
    int ret = 0;

    suc_ens = ln_dfg_nexts(dfg, trt_op, tname);
    if (!suc_ens) {
        ret = 0;
        goto end;
    }
    LN_LIST_FOREACH(en, suc_ens) {
        suc_op = en->node->data;
        if (!ln_streq(suc_op->op_arg->name, op->op_arg->name)) {
            ret = 1;
            goto end;
        }
    }
    ret = 0;

end:
    ln_list_free(suc_ens);
    return ret;
}

/* remove dsts that don't have a successor except `op` */
/* NOTE: Unable to remove the lastest-added op's dst, which usually always have
   a successor. But if not, this will waste a little GPU memory for that dst. */
static void remove_extra_dst(ln_op *trt_op, const ln_op *op, const ln_dfg *dfg)
{
    ln_list **lp;
    ln_list *tmp;
    ln_tensor_list_entry *tle;
    ln_param_entry pe;
    ln_op_arg *trt_op_arg = trt_op->op_arg;

    for (lp = &trt_op->op_arg->tensors_out; *lp;) {
        tle = (*lp)->data;
        if (have_successor_except(trt_op, tle->name, op, dfg)) {
            lp = &(*lp)->next;
            continue;
        }

        pe.arg_name = ln_strcat_delim_alloc(tle->arg_name, "shape", '_');
        trt_op_arg->params = ln_list_remove_custom_deep(trt_op_arg->params,
                                                        &pe, pe_cmp, pe_free);
        ln_free(pe.arg_name);

        pe.arg_name = ln_strcat_delim_alloc(tle->arg_name, "dtype", '_');
        trt_op_arg->params = ln_list_remove_custom_deep(trt_op_arg->params,
                                                        &pe, pe_cmp, pe_free);
        ln_free(pe.arg_name);

        tmp = *lp;
        *lp = tmp->next;
        ln_tensor_list_entry_free(tle);
        ln_free(tmp);
    }
}

static void add_trt_to_trt(ln_op *trt_op, const ln_op *op, const ln_dfg *dfg)
{
    ln_op_arg *trt_arg = trt_op->op_arg;
    ln_op_arg *op_arg = op->op_arg;
    ln_param_entry *pe;
    ln_param_entry *new_pe;
    ln_tensor_list_entry *tle;
    char *param_op_arg_name;
    int op_batch_size;
    int base_idx;

    pe = ln_param_list_find(op_arg->params, "batch_size");
    op_batch_size = pe->value_int;
    pe = ln_param_list_find(trt_arg->params, "batch_size");
    if (pe && pe->value_int == 0) {
	    pe->value_int = op_batch_size;
    } else if (pe && op_batch_size != pe->value_int) {
        ln_msg_emit(LN_MSG_ERROR,
		"batch size doesn't match among ops when converting to TensorRT: original batch_size = %d, '%s''s batch_size = %d",
                    pe->value_int, op->op_arg->name, op_batch_size);
    } else if (!pe) {
        trt_arg->params = ln_param_list_append_int(trt_arg->params,
                                                   "batch_size", op_batch_size);
    }

    LN_LIST_FOREACH(tle, op_arg->tensors_in) {
        if (ln_streqn(tle->arg_name, "src", 3))
            add_trt_src(trt_arg, op_arg, tle->name);
        else
            add_trt_weight(trt_arg, op_arg, tle->name);
    }

    remove_extra_dst(trt_op, op, dfg);

    LN_LIST_FOREACH(tle, op_arg->tensors_out) {
        add_trt_dst(trt_arg, op_arg, tle->name);
    }

    param_op_arg_name = create_arg_name_in_params(trt_arg->params, "op");
    base_idx = atoi(&param_op_arg_name[2]);
    ln_free(param_op_arg_name);

    LN_LIST_FOREACH(pe, op_arg->params) {
        if (!ln_streqn(pe->arg_name, "op", 2))
            continue;
        new_pe = ln_param_entry_copy(pe);
        ln_free(new_pe->arg_name);
        new_pe->arg_name = new_arg_name(pe->arg_name, base_idx);
        trt_arg->params = ln_list_append(trt_arg->params, new_pe);
    }
}

static int is_win_match(const ln_list *win_ops, size_t win_size)
{
    ln_op *op1;
    ln_op *op2;
    ln_list *l;
    size_t i;

    /* two tensorrt ops in a row is a match */
    for (i = 0, l = (ln_list *)win_ops; i + 1 < win_size && l && l->next;
         l = l->next, i++) {
        op1 = l->data;
        op2 = l->next->data;
        if (ln_streq(op1->op_arg->optype, "tensorrt") &&
            ln_streq(op2->op_arg->optype, "tensorrt"))
            return 1;
    }
    return 0;
}

static ln_list *cb_func_tensorrt(const ln_list *win_ops, size_t win_size,
                                 const ln_dfg *dfg, int *match)
{
    ln_list *new_ops = NULL;
    ln_list *l;
    ln_op *op;
    ln_op *trt_op;
    ln_op *no_trt_op;
    size_t i;

    *match = 0;
    if (is_win_match(win_ops, win_size))
        *match = 1;
    else
        return NULL;

    for (i = 0, l = (ln_list *)win_ops; i < win_size && l;) {
        op = l->data;
        if (!ln_streq(op->op_arg->optype, "tensorrt")) {
            i++, l = l->next;
            no_trt_op = ln_op_copy(op);
            new_ops = ln_list_append(new_ops, no_trt_op);
            continue;
        }
        trt_op = ln_op_copy(op);
        for (i++, l = l->next; i < win_size && l; i++, l = l->next) {
            op = l->data;
            if (!ln_streq(op->op_arg->optype, "tensorrt"))
                break;
            add_trt_to_trt(trt_op, op, dfg);
        }
        new_ops = ln_list_append(new_ops, trt_op);
    }

    return new_ops;
}

ln_arch ln_archimpl_tensorrt = {
    .init_func = init_tensorrt,
    .cleanup_func = cleanup_tensorrt,
    .reg_ops = ops_tensorrt,
    .ep_funcs = ep_funcs_tensorrt,
    .cb_funcs = cb_funcs_tensorrt,
    .sg_funcs = sg_funcs_tensorrt,
    .sd_funcs = sd_funcs_tensorrt,
    .arch_name = "tensorrt",
};

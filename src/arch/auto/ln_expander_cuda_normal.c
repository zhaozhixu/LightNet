/*
 * Copyright (c) 2019 Zhao Zhixu
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

 #include <assert.h>

 #include "ln_arch.h"
 #include "ln_name.h"
 #include "ln_cuda.h"

static ln_list *ep_create(const ln_op *self, const ln_dfg *dfg, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if ((({
        ln_op *next_op;
        ln_tensor_entry *te;
        int ret;

        te = ln_tensor_list_find_entry(self->op_arg->tensors_out, self->op_arg->tensor_table, "dst");
        next_op = ln_dfg_next(dfg, self, te->name);
        if (!next_op) {
            ret = 1;
        } else {
            ret = 0;
        }
        ret;
    })
    )) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table, self, "create_cpu");
        return ln_list_append(NULL, new_op);
    }

    else if ((({
        ln_op *next_op;
        ln_tensor_entry *te;
        ln_tensor_list_entry *tle_next;
        int ret;

        te = ln_tensor_list_find_entry(self->op_arg->tensors_out, self->op_arg->tensor_table, "dst");
        next_op = ln_dfg_next(dfg, self, te->name);
        if (!next_op || !ln_streq(next_op->op_arg->optype, "conv2d")) {
            ret = 0;
        } else {
            tle_next = ln_tensor_list_find_by_name(next_op->op_arg->tensors_in, te->name);
            if (!tle_next)
                ret = 0;
            else if (!ln_streq(tle_next->arg_name, "src"))
                ret = 0;
            else
                ret = 1;
        }
        ret;
    })
    ) ||
        (({
        ln_op *next_op;
        ln_tensor_entry *te;
        ln_tensor_list_entry *tle_next;
        int ret;

        te = ln_tensor_list_find_entry(self->op_arg->tensors_out, self->op_arg->tensor_table, "dst");
        next_op = ln_dfg_next(dfg, self, te->name);
        if (!next_op || !ln_streq(next_op->op_arg->optype, "relu")) {
            ret = 0;
        } else {
            tle_next = ln_tensor_list_find_by_name(next_op->op_arg->tensors_in, te->name);
            if (!tle_next)
                ret = 0;
            else if (!ln_streq(tle_next->arg_name, "src"))
                ret = 0;
            else
                ret = 1;
        }
        ret;
    })
    ) ||
        (({
        ln_op *next_op;
        ln_tensor_entry *te;
        ln_tensor_list_entry *tle_next;
        int ret;

        te = ln_tensor_list_find_entry(self->op_arg->tensors_out, self->op_arg->tensor_table, "dst");
        next_op = ln_dfg_next(dfg, self, te->name);
        if (!next_op || !ln_streq(next_op->op_arg->optype, "maxpool2d")) {
            ret = 0;
        } else {
            tle_next = ln_tensor_list_find_by_name(next_op->op_arg->tensors_in, te->name);
            if (!tle_next)
                ret = 0;
            else if (!ln_streq(tle_next->arg_name, "src"))
                ret = 0;
            else
                ret = 1;
        }
        ret;
    })
    ) ||
        (({
        ln_op *next_op;
        ln_tensor_entry *te;
        ln_tensor_list_entry *tle_next;
        int ret;

        te = ln_tensor_list_find_entry(self->op_arg->tensors_out, self->op_arg->tensor_table, "dst");
        next_op = ln_dfg_next(dfg, self, te->name);
        if (!next_op || !ln_streq(next_op->op_arg->optype, "softmax")) {
            ret = 0;
        } else {
            tle_next = ln_tensor_list_find_by_name(next_op->op_arg->tensors_in, te->name);
            if (!tle_next)
                ret = 0;
            else if (!ln_streq(tle_next->arg_name, "src"))
                ret = 0;
            else
                ret = 1;
        }
        ret;
    })
    ) ||
        (({
        ln_op *next_op;
        ln_tensor_entry *te;
        ln_tensor_list_entry *tle_next;
        int ret;

        te = ln_tensor_list_find_entry(self->op_arg->tensors_out, self->op_arg->tensor_table, "dst");
        next_op = ln_dfg_next(dfg, self, te->name);
        if (!next_op || !ln_streq(next_op->op_arg->optype, "sigmoid")) {
            ret = 0;
        } else {
            tle_next = ln_tensor_list_find_by_name(next_op->op_arg->tensors_in, te->name);
            if (!tle_next)
                ret = 0;
            else if (!ln_streq(tle_next->arg_name, "src"))
                ret = 0;
            else
                ret = 1;
        }
        ret;
    })
    ) ||
        (({
        ln_op *next_op;
        ln_tensor_entry *te;
        ln_tensor_list_entry *tle_next;
        int ret;

        te = ln_tensor_list_find_entry(self->op_arg->tensors_out, self->op_arg->tensor_table, "dst");
        next_op = ln_dfg_next(dfg, self, te->name);
        if (!next_op || !ln_streq(next_op->op_arg->optype, "concat")) {
            ret = 0;
        } else {
            tle_next = ln_tensor_list_find_by_name(next_op->op_arg->tensors_in, te->name);
            if (!tle_next)
                ret = 0;
            else if (!ln_streq(tle_next->arg_name, "src1"))
                ret = 0;
            else
                ret = 1;
        }
        ret;
    })
    ) ||
        (({
        ln_op *next_op;
        ln_tensor_entry *te;
        ln_tensor_list_entry *tle_next;
        int ret;

        te = ln_tensor_list_find_entry(self->op_arg->tensors_out, self->op_arg->tensor_table, "dst");
        next_op = ln_dfg_next(dfg, self, te->name);
        if (!next_op || !ln_streq(next_op->op_arg->optype, "concat")) {
            ret = 0;
        } else {
            tle_next = ln_tensor_list_find_by_name(next_op->op_arg->tensors_in, te->name);
            if (!tle_next)
                ret = 0;
            else if (!ln_streq(tle_next->arg_name, "src2"))
                ret = 0;
            else
                ret = 1;
        }
        ret;
    })
    ) ||
        (({
        ln_op *next_op;
        ln_tensor_entry *te;
        ln_tensor_list_entry *tle_next;
        int ret;

        te = ln_tensor_list_find_entry(self->op_arg->tensors_out, self->op_arg->tensor_table, "dst");
        next_op = ln_dfg_next(dfg, self, te->name);
        if (!next_op || !ln_streq(next_op->op_arg->optype, "transform_bboxSQD")) {
            ret = 0;
        } else {
            tle_next = ln_tensor_list_find_by_name(next_op->op_arg->tensors_in, te->name);
            if (!tle_next)
                ret = 0;
            else if (!ln_streq(tle_next->arg_name, "src_delta"))
                ret = 0;
            else
                ret = 1;
        }
        ret;
    })
    ) ||
        (({
        ln_op *next_op;
        ln_tensor_entry *te;
        ln_tensor_list_entry *tle_next;
        int ret;

        te = ln_tensor_list_find_entry(self->op_arg->tensors_out, self->op_arg->tensor_table, "dst");
        next_op = ln_dfg_next(dfg, self, te->name);
        if (!next_op || !ln_streq(next_op->op_arg->optype, "transform_bboxSQD")) {
            ret = 0;
        } else {
            tle_next = ln_tensor_list_find_by_name(next_op->op_arg->tensors_in, te->name);
            if (!tle_next)
                ret = 0;
            else if (!ln_streq(tle_next->arg_name, "src_anchor"))
                ret = 0;
            else
                ret = 1;
        }
        ret;
    })
    ) ||
        (({
        ln_op *next_op;
        ln_tensor_entry *te;
        ln_tensor_list_entry *tle_next;
        int ret;

        te = ln_tensor_list_find_entry(self->op_arg->tensors_out, self->op_arg->tensor_table, "dst");
        next_op = ln_dfg_next(dfg, self, te->name);
        if (!next_op || !ln_streq(next_op->op_arg->optype, "rearange")) {
            ret = 0;
        } else {
            tle_next = ln_tensor_list_find_by_name(next_op->op_arg->tensors_in, te->name);
            if (!tle_next)
                ret = 0;
            else if (!ln_streq(tle_next->arg_name, "src"))
                ret = 0;
            else
                ret = 1;
        }
        ret;
    })
    )) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table, self, "create_cuda");
        return ln_list_append(NULL, new_op);
    }

    else if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table, self, "create_cpu");
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_create_cuda(const ln_op *self, const ln_dfg *dfg, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        *match = 0;
        return NULL;
    }
}

static ln_list *ep_conv2d(const ln_op *self, const ln_dfg *dfg, int *match)
{
    /* auto variables */
    int last_index;

   /* replace self with new ops */
    if (((ln_param_list_find(self->op_arg->params, "padding")->value_array_int[0] != ln_param_list_find(self->op_arg->params, "padding")->value_array_int[2])) ||
        ((ln_param_list_find(self->op_arg->params, "padding")->value_array_int[1] != ln_param_list_find(self->op_arg->params, "padding")->value_array_int[3]))) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table, self, "conv2d_cuda");
        return ln_list_append(NULL, new_op);
    }

    else if (1) {
        ln_op *op_proto;
        ln_list *new_ops = NULL;

        op_proto = ln_hash_find(LN_ARCH.op_proto_table, "tensorrt");
        assert(op_proto);
        ln_op *trt = ln_op_create_with_names(op_proto, self->op_arg->tensor_table);
        new_ops = ln_list_append(new_ops, trt);

        {
            char arg_name[LN_MAX_NAME_LEN];
            last_index = ln_tensor_list_unique_arg_name(trt->op_arg->tensors_in, arg_name, "src");
            trt->op_arg->tensors_in = ln_tensor_list_append(trt->op_arg->tensors_in, arg_name, ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "src")->name);
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            last_index = ln_tensor_list_unique_arg_name(trt->op_arg->tensors_in, arg_name, "weight");
            trt->op_arg->tensors_in = ln_tensor_list_append(trt->op_arg->tensors_in, arg_name, ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "weight")->name);
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            last_index = ln_tensor_list_unique_arg_name(trt->op_arg->tensors_in, arg_name, "weight");
            trt->op_arg->tensors_in = ln_tensor_list_append(trt->op_arg->tensors_in, arg_name, ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "bias")->name);
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            last_index = ln_tensor_list_unique_arg_name(trt->op_arg->tensors_out, arg_name, "dst");
            trt->op_arg->tensors_out = ln_tensor_list_append(trt->op_arg->tensors_out, arg_name, ln_tensor_list_find_entry(self->op_arg->tensors_out, self->op_arg->tensor_table, "dst")->name);
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            last_index = ln_param_list_unique_arg_name(trt->op_arg->params, arg_name, "op");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_STRING);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                ln_free(pe->value_string);
                pe->value_string = ln_strdup("conv");
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            if (strlen("op") + strlen("_src") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
                ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                                   "op", last_index, "_src", LN_MAX_NAME_LEN);
            snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "op", last_index, "_src");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_STRING);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                ln_free(pe->value_string);
                pe->value_string = ln_strdup(ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "src")->name);
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            if (strlen("op") + strlen("_weight") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
                ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                                   "op", last_index, "_weight", LN_MAX_NAME_LEN);
            snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "op", last_index, "_weight");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_STRING);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                ln_free(pe->value_string);
                pe->value_string = ln_strdup(ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "weight")->name);
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            if (strlen("op") + strlen("_bias") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
                ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                                   "op", last_index, "_bias", LN_MAX_NAME_LEN);
            snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "op", last_index, "_bias");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_STRING);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                ln_free(pe->value_string);
                pe->value_string = ln_strdup(ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "bias")->name);
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            if (strlen("op") + strlen("_dst") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
                ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                                   "op", last_index, "_dst", LN_MAX_NAME_LEN);
            snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "op", last_index, "_dst");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_STRING);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                ln_free(pe->value_string);
                pe->value_string = ln_strdup(ln_tensor_list_find_entry(self->op_arg->tensors_out, self->op_arg->tensor_table, "dst")->name);
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            if (strlen("op") + strlen("_group") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
                ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                                   "op", last_index, "_group", LN_MAX_NAME_LEN);
            snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "op", last_index, "_group");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_NUMBER);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                pe->value_int = ln_param_list_find(self->op_arg->params, "group")->value_int;
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            if (strlen("op") + strlen("_output_c") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
                ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                                   "op", last_index, "_output_c", LN_MAX_NAME_LEN);
            snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "op", last_index, "_output_c");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_NUMBER);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                pe->value_int = ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "weight")->tensor->dims[1];
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            if (strlen("op") + strlen("_size") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
                ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                                   "op", last_index, "_size", LN_MAX_NAME_LEN);
            snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "op", last_index, "_size");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_ARRAY_NUMBER);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                pe->array_len = ln_param_list_find(self->op_arg->params, "size")->array_len;
                ln_free(pe->value_array_int);
                pe->value_array_int = ln_clone(ln_param_list_find(self->op_arg->params, "size")->value_array_int, sizeof(int)*pe->array_len);
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            if (strlen("op") + strlen("_stride") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
                ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                                   "op", last_index, "_stride", LN_MAX_NAME_LEN);
            snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "op", last_index, "_stride");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_ARRAY_NUMBER);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                pe->array_len = ln_param_list_find(self->op_arg->params, "stride")->array_len;
                ln_free(pe->value_array_int);
                pe->value_array_int = ln_clone(ln_param_list_find(self->op_arg->params, "stride")->value_array_int, sizeof(int)*pe->array_len);
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            if (strlen("op") + strlen("_padding") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
                ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                                   "op", last_index, "_padding", LN_MAX_NAME_LEN);
            snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "op", last_index, "_padding");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_ARRAY_NUMBER);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                pe->array_len = 2;
                ln_free(pe->value_array_int);
                pe->value_array_int = ln_clone((int[]){ln_param_list_find(self->op_arg->params, "padding")->value_array_int[0], ln_param_list_find(self->op_arg->params, "padding")->value_array_int[2]}, sizeof(int)*pe->array_len);
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            if (strlen("op") + strlen("_dilation") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
                ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                                   "op", last_index, "_dilation", LN_MAX_NAME_LEN);
            snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "op", last_index, "_dilation");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_ARRAY_NUMBER);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                pe->array_len = ln_param_list_find(self->op_arg->params, "dilation")->array_len;
                ln_free(pe->value_array_int);
                pe->value_array_int = ln_clone(ln_param_list_find(self->op_arg->params, "dilation")->value_array_int, sizeof(int)*pe->array_len);
            }
        }

        {
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, "batch_size");
            {
                pe->value_int = ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "src")->tensor->dims[0];
            }
        }

        return new_ops;
    }
}

static ln_list *ep_batchnorm(const ln_op *self, const ln_dfg *dfg, int *match)
{
    /* auto variables */
    int last_index;

   /* replace self with new ops */
    if (1) {
        ln_op *op_proto;
        ln_list *new_ops = NULL;

        op_proto = ln_hash_find(LN_ARCH.op_proto_table, "bn2scale_wts_cpu");
        assert(op_proto);
        ln_op *bwc = ln_op_create_with_names(op_proto, self->op_arg->tensor_table);
        new_ops = ln_list_append(new_ops, bwc);

        op_proto = ln_hash_find(LN_ARCH.op_proto_table, "tensorrt");
        assert(op_proto);
        ln_op *trt = ln_op_create_with_names(op_proto, self->op_arg->tensor_table);
        new_ops = ln_list_append(new_ops, trt);

        {
            ln_tensor_list_entry *tle = ln_tensor_list_find_by_arg_name(bwc->op_arg->tensors_in,
                                                                        "src_mean");
            ln_free(tle->name);
            tle->name = ln_strdup(ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "mean")->name);
        }

        {
            ln_tensor_list_entry *tle = ln_tensor_list_find_by_arg_name(bwc->op_arg->tensors_in,
                                                                        "src_var");
            ln_free(tle->name);
            tle->name = ln_strdup(ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "var")->name);
        }

        {
            ln_tensor_list_entry *tle = ln_tensor_list_find_by_arg_name(bwc->op_arg->tensors_in,
                                                                        "src_scale");
            ln_free(tle->name);
            tle->name = ln_strdup(ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "scale")->name);
        }

        {
            ln_tensor_list_entry *tle = ln_tensor_list_find_by_arg_name(bwc->op_arg->tensors_in,
                                                                        "src_offset");
            ln_free(tle->name);
            tle->name = ln_strdup(ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "offset")->name);
        }

        {
            ln_param_entry *pe = ln_param_list_find(bwc->op_arg->params, "epsilon");
            {
                pe->value_float = ln_param_list_find(self->op_arg->params, "epsilon")->value_float;
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            last_index = ln_tensor_list_unique_arg_name(trt->op_arg->tensors_in, arg_name, "src");
            trt->op_arg->tensors_in = ln_tensor_list_append(trt->op_arg->tensors_in, arg_name, ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "src")->name);
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            last_index = ln_tensor_list_unique_arg_name(trt->op_arg->tensors_out, arg_name, "dst");
            trt->op_arg->tensors_out = ln_tensor_list_append(trt->op_arg->tensors_out, arg_name, ln_tensor_list_find_entry(self->op_arg->tensors_out, self->op_arg->tensor_table, "dst")->name);
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            last_index = ln_tensor_list_unique_arg_name(trt->op_arg->tensors_out, arg_name, "weight");
            trt->op_arg->tensors_out = ln_tensor_list_append(trt->op_arg->tensors_out, arg_name, ln_tensor_list_find_entry(bwc->op_arg->tensors_out, bwc->op_arg->tensor_table, "dst_shift")->name);
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            last_index = ln_tensor_list_unique_arg_name(trt->op_arg->tensors_out, arg_name, "weight");
            trt->op_arg->tensors_out = ln_tensor_list_append(trt->op_arg->tensors_out, arg_name, ln_tensor_list_find_entry(bwc->op_arg->tensors_out, bwc->op_arg->tensor_table, "dst_scale")->name);
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            last_index = ln_tensor_list_unique_arg_name(trt->op_arg->tensors_out, arg_name, "weight");
            trt->op_arg->tensors_out = ln_tensor_list_append(trt->op_arg->tensors_out, arg_name, ln_tensor_list_find_entry(bwc->op_arg->tensors_out, bwc->op_arg->tensor_table, "dst_power")->name);
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            last_index = ln_param_list_unique_arg_name(trt->op_arg->params, arg_name, "op");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_STRING);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                ln_free(pe->value_string);
                pe->value_string = ln_strdup("scale");
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            if (strlen("op") + strlen("_src") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
                ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                                   "op", last_index, "_src", LN_MAX_NAME_LEN);
            snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "op", last_index, "_src");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_STRING);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                ln_free(pe->value_string);
                pe->value_string = ln_strdup(ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "src")->name);
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            if (strlen("op") + strlen("_dst") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
                ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                                   "op", last_index, "_dst", LN_MAX_NAME_LEN);
            snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "op", last_index, "_dst");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_STRING);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                ln_free(pe->value_string);
                pe->value_string = ln_strdup(ln_tensor_list_find_entry(self->op_arg->tensors_out, self->op_arg->tensor_table, "dst")->name);
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            if (strlen("op") + strlen("_shift") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
                ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                                   "op", last_index, "_shift", LN_MAX_NAME_LEN);
            snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "op", last_index, "_shift");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_STRING);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                ln_free(pe->value_string);
                pe->value_string = ln_strdup(ln_tensor_list_find_entry(bwc->op_arg->tensors_out, bwc->op_arg->tensor_table, "dst_shift")->name);
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            if (strlen("op") + strlen("_scale") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
                ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                                   "op", last_index, "_scale", LN_MAX_NAME_LEN);
            snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "op", last_index, "_scale");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_STRING);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                ln_free(pe->value_string);
                pe->value_string = ln_strdup(ln_tensor_list_find_entry(bwc->op_arg->tensors_out, bwc->op_arg->tensor_table, "dst_scale")->name);
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            if (strlen("op") + strlen("_power") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
                ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                                   "op", last_index, "_power", LN_MAX_NAME_LEN);
            snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "op", last_index, "_power");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_STRING);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                ln_free(pe->value_string);
                pe->value_string = ln_strdup(ln_tensor_list_find_entry(bwc->op_arg->tensors_out, bwc->op_arg->tensor_table, "dst_power")->name);
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            if (strlen("op") + strlen("_scale_mode") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
                ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                                   "op", last_index, "_scale_mode", LN_MAX_NAME_LEN);
            snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "op", last_index, "_scale_mode");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_STRING);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                ln_free(pe->value_string);
                pe->value_string = ln_strdup("kCHANNEL");
            }
        }

        {
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, "batch_size");
            {
                pe->value_int = ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "src")->tensor->dims[0];
            }
        }

        return new_ops;
    }
}

static ln_list *ep_concat(const ln_op *self, const ln_dfg *dfg, int *match)
{
    /* auto variables */
    int last_index;

   /* replace self with new ops */
    if (1) {
        ln_op *op_proto;
        ln_list *new_ops = NULL;

        op_proto = ln_hash_find(LN_ARCH.op_proto_table, "tensorrt");
        assert(op_proto);
        ln_op *trt = ln_op_create_with_names(op_proto, self->op_arg->tensor_table);
        new_ops = ln_list_append(new_ops, trt);

        {
            char arg_name[LN_MAX_NAME_LEN];
            last_index = ln_tensor_list_unique_arg_name(trt->op_arg->tensors_in, arg_name, "src");
            trt->op_arg->tensors_in = ln_tensor_list_append(trt->op_arg->tensors_in, arg_name, ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "src1")->name);
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            last_index = ln_tensor_list_unique_arg_name(trt->op_arg->tensors_in, arg_name, "src");
            trt->op_arg->tensors_in = ln_tensor_list_append(trt->op_arg->tensors_in, arg_name, ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "src2")->name);
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            last_index = ln_tensor_list_unique_arg_name(trt->op_arg->tensors_out, arg_name, "dst");
            trt->op_arg->tensors_out = ln_tensor_list_append(trt->op_arg->tensors_out, arg_name, ln_tensor_list_find_entry(self->op_arg->tensors_out, self->op_arg->tensor_table, "dst")->name);
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            last_index = ln_param_list_unique_arg_name(trt->op_arg->params, arg_name, "op");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_STRING);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                ln_free(pe->value_string);
                pe->value_string = ln_strdup("concat");
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            if (strlen("op") + strlen("_src1") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
                ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                                   "op", last_index, "_src1", LN_MAX_NAME_LEN);
            snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "op", last_index, "_src1");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_STRING);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                ln_free(pe->value_string);
                pe->value_string = ln_strdup(ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "src1")->name);
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            if (strlen("op") + strlen("_src2") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
                ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                                   "op", last_index, "_src2", LN_MAX_NAME_LEN);
            snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "op", last_index, "_src2");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_STRING);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                ln_free(pe->value_string);
                pe->value_string = ln_strdup(ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "src2")->name);
            }
        }

        {
            char arg_name[LN_MAX_NAME_LEN];
            if (strlen("op") + strlen("_axis") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
                ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                                   "op", last_index, "_axis", LN_MAX_NAME_LEN);
            snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "op", last_index, "_axis");
            trt->op_arg->params = ln_param_list_append_empty(trt->op_arg->params, arg_name, LN_PARAM_NUMBER);
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, arg_name);
            {
                pe->value_int = ln_param_list_find(self->op_arg->params, "axis")->value_int;
            }
        }

        {
            ln_param_entry *pe = ln_param_list_find(trt->op_arg->params, "batch_size");
            {
                pe->value_int = ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "src1")->tensor->dims[0];
            }
        }

        return new_ops;
    }
}

static ln_list *ep_print(const ln_op *self, const ln_dfg *dfg, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (((ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "src")->mtype == LN_MEM_CPU))) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table, self, "print_cpu");
        return ln_list_append(NULL, new_op);
    }

    else if (((ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "src")->mtype == LN_MEM_CUDA))) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table, self, "print_cuda");
        return ln_list_append(NULL, new_op);
    }

    else if (1) {
        ln_msg_inter_error("ep_print(): optype 'print''s 'src' is either of LN_MEM_CPU or LN_MEM_CUDA");
        return NULL;
    }
}

static ln_list *ep_fprint(const ln_op *self, const ln_dfg *dfg, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (((ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "src")->mtype == LN_MEM_CPU))) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table, self, "fprint_cpu");
        return ln_list_append(NULL, new_op);
    }

    else if (((ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "src")->mtype == LN_MEM_CUDA))) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table, self, "fprint_cuda");
        return ln_list_append(NULL, new_op);
    }

    else if (1) {
        ln_msg_inter_error("ep_fprint(): optype 'fprint''s 'src' is either of LN_MEM_CPU or LN_MEM_CUDA");
        return NULL;
    }
}

static ln_hash_init_entry init_ep_funcs[] = {
    {"create", ep_create},
    {"create_cuda", ep_create_cuda},
    {"conv2d", ep_conv2d},
    {"batchnorm", ep_batchnorm},
    {"concat", ep_concat},
    {"print", ep_print},
    {"fprint", ep_fprint},
    LN_HASH_INIT_ENTRY_NULL
};
static ln_hash *ep_funcs_hash = NULL;

void ln_expander_init_cuda_normal(void **context_p)
{
    ep_funcs_hash = ln_hash_create(ln_str_hash, ln_str_cmp, NULL, NULL);
    ln_hash_init(ep_funcs_hash, init_ep_funcs);
}

void ln_expander_cleanup_cuda_normal(void **context_p)
{
    ln_hash_free(ep_funcs_hash);
}

ln_list *ln_expander_cuda_normal(const ln_op *self, const ln_dfg *dfg, int *match)
{
    ln_expander_func  ep_func;
    ln_list          *new_ops;
    void             *value;

    if (!ln_hash_find_extended(ep_funcs_hash, self->op_arg->optype, NULL, &value))
        ln_msg_inter_error("unsupported optype \"%s\" for cuda_normal expander",
                           self->op_arg->optype);

    ep_func = value;
    new_ops = ep_func(self, dfg, match);

    return new_ops;
}

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

#include <string.h>
#include <assert.h>
#include "cJSON.h"
#include "ln_json.h"

#define PARSE_JSON_ERROR(varg...) ln_error_emit(LN_ERROR, ##varg)

static ln_list *parse_array_value(const cJSON *array_json,
                                  const cJSON *name_json,
                                  const cJSON *param_arg_name_json,
                                  ln_list *param_list)
{
    ln_param_type first_type = LN_PARAM_INVALID;
    ln_param_type type = LN_PARAM_INVALID;
    cJSON *element_json;
    int array_len = cJSON_GetArraySize(array_json);
    char **array_string = NULL;
    double *array_number = NULL;
    ln_bool *array_bool = NULL;

    /* check the consistence of elements' types */
    int idx = 0;
    cJSON_ArrayForEach(element_json, array_json) {
        if (cJSON_IsNumber(element_json)) {
            type = LN_PARAM_NUMBER;
        }
        else if (cJSON_IsString(element_json)) {
            type = LN_PARAM_STRING;
        }
        else if (cJSON_IsTrue(element_json)) {
            type = LN_PARAM_BOOL;
        }
        else if (cJSON_IsFalse(element_json)) {
            type = LN_PARAM_BOOL;
        }
        else {
            PARSE_JSON_ERROR("op \"%s\"'s param \"%s\"'s value has an unsupported JSON array element type at element index %d",
                             name_json->valuestring,
                             param_arg_name_json->valuestring, idx);
        }
        if (first_type == LN_PARAM_INVALID) {
            first_type = type;
            switch (first_type) {
            case LN_PARAM_STRING:
                array_string = ln_alloc(sizeof(char *)*array_len);
                memset(array_string, 0, sizeof(char *)*array_len);
                break;
            case LN_PARAM_NUMBER:
                array_number = ln_alloc(sizeof(double)*array_len);
                break;
            case LN_PARAM_BOOL:
                array_bool = ln_alloc(sizeof(ln_bool)*array_len);
                break;
            default:
                assert(0 && "handled before, shouldn't get here");
                break;
            }
        }
        else if (first_type != type) {
            PARSE_JSON_ERROR("op \"%s\"'s param \"%s\"'s value is inconsistent among its elements' JSON type at element index %d",
                              name_json->valuestring,
                              param_arg_name_json->valuestring, idx);
        }
        switch (type) {
        case LN_PARAM_STRING:
            array_string[idx] =
                ln_alloc(sizeof(char)*(strlen(element_json->valuestring)+1));
            strcpy(array_string[idx], element_json->valuestring);
            break;
        case LN_PARAM_NUMBER:
            array_number[idx] = element_json->valuedouble;
            break;
        case LN_PARAM_BOOL:
            array_bool[idx] = cJSON_IsTrue(element_json) ? LN_TRUE : LN_FALSE;
            break;
        default:
            assert(0 && "handled before, shouldn't get here");
            break;
        }
        idx++;
    }

    assert(idx == array_len);
    switch (first_type) {
    case LN_PARAM_STRING:
        param_list = ln_param_list_append_array_string(param_list,
                                                       param_arg_name_json->valuestring,
                                                       array_len, array_string);
        break;
    case LN_PARAM_NUMBER:
        param_list = ln_param_list_append_array_number(param_list,
                                                       param_arg_name_json->valuestring,
                                                       array_len, array_number);
        break;
    case LN_PARAM_BOOL:
        param_list = ln_param_list_append_array_bool(param_list,
                                                     param_arg_name_json->valuestring,
                                                     array_len, array_bool);
        break;
    case LN_PARAM_INVALID:
        assert(array_len == 0);
        PARSE_JSON_ERROR("op \"%s\"'s param \"%s\"'s value is an empty array",
                         name_json->valuestring,
                         param_arg_name_json->valuestring);
        break;
    default:
        assert(0 && "handled before, shouldn't get here");
        break;
    }

    return param_list;
}

static ln_op *parse_op(const cJSON *op_json, ln_context *ctx, int idx)
{
    ln_op *op, *proto_op;
    ln_list *tensors_in = NULL;
    ln_list *tensors_out = NULL;
    ln_list *params = NULL;

    cJSON *name_json = cJSON_GetObjectItem(op_json, "name");
    cJSON *optype_json = cJSON_GetObjectItem(op_json, "optype");
    cJSON *tensors_in_json = cJSON_GetObjectItem(op_json, "tensors_in");
    cJSON *tensors_out_json = cJSON_GetObjectItem(op_json, "tensors_out");
    cJSON *params_json = cJSON_GetObjectItem(op_json, "params");

    if (!name_json)
        PARSE_JSON_ERROR("ops[%d] doesn't have a \"name\" key", idx);

    if (!cJSON_IsString(name_json))
        PARSE_JSON_ERROR("ops[%d]'s name is not a String", idx);

    if (!optype_json)
        PARSE_JSON_ERROR("op \"%s\" doesn't have an \"optype\" key",
                         name_json->valuestring);

    if (!cJSON_IsString(optype_json))
        PARSE_JSON_ERROR("op \"%s\"'s \"optype\" is not a String",
                         name_json->valuestring);

    if (!tensors_in_json)
        PARSE_JSON_ERROR("op \"%s\" doesn't have a \"tensors_in\" key",
                         name_json->valuestring);

    if (!tensors_out_json)
        PARSE_JSON_ERROR("op \"%s\" doesn't have a \"tensors_out\" key",
                         name_json->valuestring);

    if (!cJSON_IsArray(tensors_in_json))
        PARSE_JSON_ERROR("op \"%s\"'s \"tensors_in\" is not an Array",
                         name_json->valuestring);

    if (!cJSON_IsArray(tensors_out_json))
        PARSE_JSON_ERROR("op \"%s\"'s \"tensors_out\" is not an Array",
                         name_json->valuestring);

    if (!params_json)
        PARSE_JSON_ERROR("op \"%s\" doesn't have a \"params\" key",
                         name_json->valuestring);

    if (!cJSON_IsArray(params_json))
        PARSE_JSON_ERROR("op \"%s\"'s \"params\" is not an Array",
                         name_json->valuestring);

    cJSON *tensor_json, *param_json;
    cJSON *tensor_arg_name_json, *tensor_name_json;
    cJSON *param_arg_name_json, *param_value_json;
    int i = 0;
    cJSON_ArrayForEach(tensor_json, tensors_in_json) {
        tensor_arg_name_json = cJSON_GetObjectItem(tensor_json, "arg_name");
        tensor_name_json = cJSON_GetObjectItem(tensor_json, "name");
        if (!tensor_arg_name_json)
            PARSE_JSON_ERROR("one of op \"%s\"'s input tensors doesn't have an \"arg_name\" key at tensor index %d",
                             name_json->valuestring, i);

        if (!cJSON_IsString(tensor_arg_name_json))
            PARSE_JSON_ERROR("one of op \"%s\"'s input tensors's arg_name is not a String at tensor index %d",
                             name_json->valuestring, i);

        if (!tensor_name_json)
            PARSE_JSON_ERROR("op \"%s\"'s \"%s\" tensor doesn't have a \"name\" key",
                             name_json->valuestring,
                             tensor_arg_name_json->valuestring);

        if (!cJSON_IsString(tensor_name_json))
            PARSE_JSON_ERROR("op \"%s\"'s \"%s\" tensor's name is not a String",
                             name_json->valuestring,
                             tensor_arg_name_json->valuestring);

        tensors_in = ln_tensor_list_append(tensors_in,
                                           tensor_arg_name_json->valuestring,
                                           tensor_name_json->valuestring);
        i++;
    }
    i = 0;
    cJSON_ArrayForEach(tensor_json, tensors_out_json) {
        tensor_arg_name_json = cJSON_GetObjectItem(tensor_json, "arg_name");
        tensor_name_json = cJSON_GetObjectItem(tensor_json, "name");
        if (!tensor_arg_name_json)
            PARSE_JSON_ERROR("one of op \"%s\"'s output tensors doesn't have an \"arg_name\" key at tensor index %d",
                             name_json->valuestring, i);

        if (!cJSON_IsString(tensor_arg_name_json))
            PARSE_JSON_ERROR("one of op \"%s\"'s output tensors's arg_name is not a String at tensor index %d",
                             name_json->valuestring, i);

        if (!tensor_name_json)
            PARSE_JSON_ERROR("op \"%s\"'s \"%s\" tensor doesn't have a \"name\" key",
                             name_json->valuestring,
                             tensor_arg_name_json->valuestring);

        if (!cJSON_IsString(tensor_name_json))
            PARSE_JSON_ERROR("op \"%s\"'s \"%s\" tensor's name is not a String",
                             name_json->valuestring,
                             tensor_arg_name_json->valuestring);

        tensors_out = ln_tensor_list_append(tensors_out,
                                            tensor_arg_name_json->valuestring,
                                            tensor_name_json->valuestring);
        i++;
    }
    i = 0;
    cJSON_ArrayForEach(param_json, params_json) {
        param_arg_name_json = cJSON_GetObjectItem(param_json, "arg_name");
        if (!param_arg_name_json)
            PARSE_JSON_ERROR("one of op \"%s\"'s params doesn't have an \"arg_name\" key at param index %d",
                             name_json->valuestring, i);

        if (!cJSON_IsString(param_arg_name_json))
            PARSE_JSON_ERROR("one of op \"%s\"'s params's arg_name is not a String at param index %d",
                             name_json->valuestring);

        param_value_json = cJSON_GetObjectItem(param_json, "value");

        if (!param_value_json)
            PARSE_JSON_ERROR("op \"%s\"'s \"%s\" param doesn't have a \"value\" key",
                             name_json->valuestring,
                             param_arg_name_json->valuestring);

        if (cJSON_IsNumber(param_value_json)) {
            params = ln_param_list_append_number(params,
                                                 param_arg_name_json->valuestring,
                                                 param_value_json->valuedouble);
        }
        else if (cJSON_IsString(param_value_json)) {
            params = ln_param_list_append_string(params,
                                                 param_arg_name_json->valuestring,
                                                 param_value_json->valuestring);
        }
        else if (cJSON_IsTrue(param_value_json)) {
            params = ln_param_list_append_bool(params,
                                               param_arg_name_json->valuestring,
                                               LN_TRUE);
        }
        else if (cJSON_IsFalse(param_value_json)) {
            params = ln_param_list_append_bool(params,
                                               param_arg_name_json->valuestring,
                                               LN_FALSE);
        }
        else if (cJSON_IsNull(param_value_json)) {
            params = ln_param_list_append_null(params,
                                               param_arg_name_json->valuestring);
        }
        else if (cJSON_IsArray(param_value_json)) {
            params = parse_array_value(param_value_json, name_json,
                                       param_arg_name_json, params);
        }
        else {
            PARSE_JSON_ERROR("op \"%s\"'s param \"%s\"'s value is an unsupported JSON type",
                             name_json->valuestring,
                             param_arg_name_json->valuestring);
        }
        i++;
    }
    proto_op = ln_hash_find(LN_ARCH.op_proto_table, optype_json->valuestring);
    if (!proto_op) {
        PARSE_JSON_ERROR("op \"%s\"'s optype \"%s\" is not registered",
                         name_json->valuestring,
                         optype_json->valuestring);
    }

    op = ln_op_create_from_proto(proto_op, name_json->valuestring, tensors_in,
                                 tensors_out, params, ctx->tensor_table);

    int ret = ln_op_table_insert(ctx->op_table, op);
    if (!ret)
        PARSE_JSON_ERROR("op \"%s\"'s name is duplicated");

    return op;
}

/* static int line_num(int index, int *newline_indices) */
/* { */
/*      int *array = newline_indices + 1; */
/*      int len = newline_indices[0]; */
/*      int low = 0, high = len - 1, mid; */

/*      while (low <= high) { */
/*           mid = (high - low) / 2 + low; */
/*           if (array[mid] < index) */
/*                low = mid + 1; */
/*           else if (array[mid] > index) */
/*                high = mid - 1; */
/*           else */
/*                break; */
/*      } */

/*      return array[mid] <= index ? mid + 1 : mid; */
/* } */

#define RECORD_NEWLINE                                          \
    do {                                                        \
        if (i >= array_size) {                                  \
            array_size *= 2;                                    \
            array = ln_realloc(array, sizeof(int)*array_size);  \
        }                                                       \
        array[0] = i;                                           \
        array[i++] = p - json_str;                              \
    } while (0)

/* Replace comments to blanks, and record newlines' positions.
   The first element of returned array is the number of newlines,
   and others are positions of newlines. */
static int *preprocess(char *json_str)
{
    int *array;
    int array_size = 256;
    int i = 1;

    array = ln_realloc(NULL, sizeof(int)*array_size);

    for (char *p = json_str; *p; p++) {
        if (*p == '\n') {
            RECORD_NEWLINE;
            continue;
        }
        if (*p == '\"') {
            for (p++; *p && *p != '\"'; p++) {
                if (*p == '\n')
                    RECORD_NEWLINE;
            }
            if (!*p)
                break;
            continue;
        }
        if (*p == '/' && *(p+1) == '/') {
            for (; *p && *p != '\n'; p++)
                *p = ' ';
            if (!*p)
                break;
            RECORD_NEWLINE;
            continue;
        }
        if (*p == '/' && *(p+1) == '*') {
            char *mark = p;  /* TODO: strdup */
            for (; *p && !(*p == '*' && *(p+1) == '/'); p++) {
                if (*p == '\n') {
                    RECORD_NEWLINE;
                    continue;
                }
                *p = ' ';
            }
            if (!*p)
                ln_error_emit(LN_ERROR, "unterminated comment: %s", mark);
            *p++ = ' ';
            *p = ' ';
            continue;
        }
    }

    return array;
}

ln_list *ln_json_parse(char *json_str, ln_context *ctx)
{
    int *newline_indices;
    const cJSON *ops_json;
    const cJSON *op_json;
    cJSON *json;
    ln_list *ops = NULL;
    ln_op *op;

    newline_indices = preprocess(json_str);
    json = cJSON_Parse(json_str);
    if (!json)
        PARSE_JSON_ERROR("syntax error in %s", cJSON_GetErrorPtr());

    ops_json = cJSON_GetObjectItem(json, "ops");

    if (!ops_json)
        PARSE_JSON_ERROR("top object should have an \"ops\" item");

    if (!cJSON_IsArray(ops_json))
        PARSE_JSON_ERROR("item \"ops\" has to be an Array");

    int i = 0;
    cJSON_ArrayForEach(op_json, ops_json) {
        op = parse_op(op_json, ctx, i);
        ops = ln_list_append(ops, op);
        i++;
    }

    ctx->ops = ops;
    cJSON_Delete(json);
    return ops;
}

ln_list *ln_json_parse_file(const char *file, ln_context *ctx)
{
    char *str;
    ln_list *ops;

    str = ln_read_text(file);
    ops = ln_json_parse(str, ctx);
    ln_free(str);

    return ops;
}

#define PRINT_JSON_ERROR ln_error_emit(LN_ERROR, "ln_json_print_ops() failed")

static void add_tensors(cJSON *tensors_json, ln_list *tensors,
                        ln_hash *tensor_table)
{
    ln_tensor_list_entry *tle;
    ln_tensor_entry *te;
    cJSON *tensor_json = NULL;
    cJSON *item = NULL;

    LN_LIST_FOREACH(tle, tensors) {
        tensor_json = cJSON_CreateObject();

        item = cJSON_AddStringToObject(tensor_json, "arg_name", tle->arg_name);
        if (!item)
            PRINT_JSON_ERROR;
        item = cJSON_AddStringToObject(tensor_json, "name", tle->name);
        if (!item)
            PRINT_JSON_ERROR;

        /* optionally print tensor shape */
        if (tensor_table) {
            te = ln_tensor_table_find(tensor_table, tle->name);
            if (te) {
                item = cJSON_CreateIntArray(te->tensor->dims, te->tensor->ndim);
                if (!item)
                    PRINT_JSON_ERROR;
                cJSON_AddItemToObject(tensor_json, "dims", item);
            }
        }

        cJSON_AddItemToArray(tensors_json, tensor_json);
    }
}

static void add_params(cJSON *params_json, ln_list *params)
{
    ln_param_entry *pe;
    cJSON *param_json = NULL;
    cJSON *item = NULL;

    LN_LIST_FOREACH(pe, params) {
        param_json = cJSON_CreateObject();
        if (!param_json)
            PRINT_JSON_ERROR;

        item = cJSON_AddStringToObject(param_json, "arg_name", pe->arg_name);
        if (!item)
            PRINT_JSON_ERROR;

        switch (pe->type) {
        case LN_PARAM_BOOL:
            item = cJSON_AddBoolToObject(param_json, "value",
                                         pe->value_bool ?
                                         cJSON_True : cJSON_False);
            break;
        case LN_PARAM_NULL:
            item = cJSON_AddNullToObject(param_json, "value");
            break;
        case LN_PARAM_NUMBER:
            item = cJSON_AddNumberToObject(param_json, "value",
                                           pe->value_double);
            break;
        case LN_PARAM_STRING:
            item = cJSON_AddStringToObject(param_json, "value",
                                           pe->value_string);
            break;
        case LN_PARAM_ARRAY_BOOL:
            /* TODO: add cJSON_CreateBoolArray() */
            item = cJSON_CreateIntArray((int *)pe->value_array_bool,
                                        pe->array_len);
            cJSON_AddItemToObject(param_json, "value", item);
            break;
        case LN_PARAM_ARRAY_NUMBER:
            item = cJSON_CreateDoubleArray(pe->value_array_double,
                                           pe->array_len);
            cJSON_AddItemToObject(param_json, "value", item);
            break;
        case LN_PARAM_ARRAY_STRING:
            item = cJSON_CreateStringArray((const char**)pe->value_array_string,
                                           pe->array_len);
            cJSON_AddItemToObject(param_json, "value", item);
            break;
        default:
            assert(0 && "unsupported ln_param_type");
            break;
        }
        if (!item)
            PRINT_JSON_ERROR;

        cJSON_AddItemToArray(params_json, param_json);
    }
}

char *ln_json_create_json_str(const ln_context *ctx)
{
    char *str = NULL;
    ln_op *op;
    cJSON *json = NULL;
    cJSON *ops_json = NULL;
    cJSON *op_json = NULL;
    cJSON *item = NULL;
    int i;

    json = cJSON_CreateObject();
    if (!json)
        PRINT_JSON_ERROR;
    ops_json = cJSON_AddArrayToObject(json, "ops");
    if (!ops_json)
        PRINT_JSON_ERROR;

    LN_LIST_FOREACH(op, ctx->ops) {
        op_json = cJSON_CreateObject();
        if (!op_json)
            PRINT_JSON_ERROR;

        item = cJSON_AddStringToObject(op_json, "name", op->op_arg->name);
        if (!item)
            PRINT_JSON_ERROR;

        item = cJSON_AddStringToObject(op_json, "optype", op->op_arg->optype);
        if (!item)
            PRINT_JSON_ERROR;

        item = cJSON_AddArrayToObject(op_json, "tensors_in");
        if (!item)
            PRINT_JSON_ERROR;
        add_tensors(item, op->op_arg->tensors_in, op->op_arg->tensor_table);

        item = cJSON_AddArrayToObject(op_json, "tensors_out");
        if (!item)
            PRINT_JSON_ERROR;
        add_tensors(item, op->op_arg->tensors_out, op->op_arg->tensor_table);

        item = cJSON_AddArrayToObject(op_json, "params");
        if (!item)
            PRINT_JSON_ERROR;
        add_params(item, op->op_arg->params);

        cJSON_AddItemToArray(ops_json, op_json);
    }

    str = cJSON_Print(json);
    if (!str)
        PRINT_JSON_ERROR;

    cJSON_Delete(json);
    return str;
}

void ln_json_fprint(FILE *fp, const ln_context *ctx)
{
    char *str;

    str = ln_json_create_json_str(ctx);
    fprintf(fp, "%s\n", str);
    ln_free(str);
}

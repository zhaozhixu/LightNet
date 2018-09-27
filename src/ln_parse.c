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
#include "ln_parse.h"
#include "ln_op.h"
#include "cJSON.h"

static ln_list *parse_array_value(const cJSON *array_json,
                                  const cJSON *name_json,
                                  const cJSON *param_arg_name_json,
                                  ln_list *param_list,
                                  ln_error **error)
{
     ln_param_type first_type = LN_PARAM_INVALID;
     ln_param_type type;
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
	       *error = ln_error_create(LN_ERROR,
					"op \"%s\"'s param \"%s\"'s value has an unsupported JSON array element type at element index %d",
					name_json->valuestring,
					param_arg_name_json->valuestring, idx);
	       goto end;
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
	       *error = ln_error_create(LN_ERROR,
					"op \"%s\"'s param \"%s\"'s value is inconsistent among its elements' JSON type at element index %d",
					name_json->valuestring,
					param_arg_name_json->valuestring, idx);
	       goto end;
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
	  *error = ln_error_create(LN_ERROR,
				   "op \"%s\"'s param \"%s\"'s value is an empty array",
				   name_json->valuestring,
				   param_arg_name_json->valuestring);
	  goto end;
     default:
	  assert(0 && "handled before, shouldn't get here");
	  break;
     }

end:
     if (first_type == LN_PARAM_STRING) {
	  for (; idx; idx--)
	       ln_free(array_string[idx-1]);
     }
     ln_free(array_string);
     ln_free(array_number);
     ln_free(array_bool);
     return param_list;
}

static ln_op *parse_op(const cJSON *op_json, ln_list *registered_ops,
                       ln_hash *tensor_table, int idx, ln_error **error)
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
     if (!name_json) {
	  *error = ln_error_create(LN_ERROR,
				   "one of the ops doesn't have a \"name\" key at op index %d",
				   idx);
	  goto err;
     }
     if (!cJSON_IsString(name_json)) {
	  *error = ln_error_create(LN_ERROR,
				   "one of the ops's name is not a String at op index %d",
				   idx);
	  goto err;
     }
     if (!optype_json) {
	  *error = ln_error_create(LN_ERROR,
				   "op \"%s\" doesn't have an \"optype\" key",
				   name_json->valuestring);
	  goto err;
     }
     if (!cJSON_IsString(optype_json)) {
	  *error = ln_error_create(LN_ERROR,
				   "op \"%s\"'s \"optype\" is not a String",
				   name_json->valuestring);
	  goto err;
     }
     if (!tensors_in_json) {
	  *error = ln_error_create(LN_ERROR,
				   "op \"%s\" doesn't have a \"tensors_in\" key",
				   name_json->valuestring);
	  goto err;
     }
     if (!tensors_out_json) {
	  *error = ln_error_create(LN_ERROR,
				   "op \"%s\" doesn't have a \"tensors_out\" key",
				   name_json->valuestring);
	  goto err;
     }
     if (!cJSON_IsArray(tensors_in_json)) {
	  *error = ln_error_create(LN_ERROR,
				   "op \"%s\"'s \"tensors_in\" is not an Array",
				   name_json->valuestring);
	  goto err;
     }
     if (!cJSON_IsArray(tensors_out_json)) {
	  *error = ln_error_create(LN_ERROR,
				   "op \"%s\"'s \"tensors_out\" is not an Array",
				   name_json->valuestring);
	  goto err;
     }
     if (!params_json) {
	  *error = ln_error_create(LN_ERROR,
				   "op \"%s\" doesn't have a \"params\" key",
				   name_json->valuestring);
	  goto err;
     }
     if (!cJSON_IsArray(params_json)) {
	  *error = ln_error_create(LN_ERROR,
				   "op \"%s\"'s \"params\" is not an Array",
				   name_json->valuestring);
	  goto err;
     }

     cJSON *tensor_json, *param_json;
     cJSON *tensor_arg_name_json, *tensor_name_json;
     cJSON *param_arg_name_json, *param_value_json;
     int i = 0;
     cJSON_ArrayForEach(tensor_json, tensors_in_json) {
	  tensor_arg_name_json = cJSON_GetObjectItem(tensor_json, "arg_name");
	  tensor_name_json = cJSON_GetObjectItem(tensor_json, "name");
	  if (!tensor_arg_name_json) {
	       *error = ln_error_create(LN_ERROR,
					"one of op \"%s\"'s input tensors doesn't have an \"arg_name\" key at tensor index %d",
					name_json->valuestring, i);
	       goto err;
	  }
	  if (!cJSON_IsString(tensor_arg_name_json)) {
	       *error = ln_error_create(LN_ERROR,
					"one of op \"%s\"'s input tensors's arg_name is not a String at tensor index %d",
					name_json->valuestring, i);
	       goto err;
	  }
	  if (!tensor_name_json) {
	       *error = ln_error_create(LN_ERROR,
					"op \"%s\"'s \"%s\" tensor doesn't have a \"name\" key",
					name_json->valuestring,
					tensor_arg_name_json->valuestring);
	       goto err;
	  }
	  if (!cJSON_IsString(tensor_name_json)) {
	       *error = ln_error_create(LN_ERROR,
					"op \"%s\"'s \"%s\" tensor's name is not a String",
					name_json->valuestring,
					tensor_arg_name_json->valuestring);
	       goto err;
	  }
	  tensors_in = ln_tensor_list_append(tensors_in,
                                             tensor_arg_name_json->valuestring,
                                             tensor_name_json->valuestring);
	  i++;
     }
     i = 0;
     cJSON_ArrayForEach(tensor_json, tensors_out_json) {
	  tensor_arg_name_json = cJSON_GetObjectItem(tensor_json, "arg_name");
	  tensor_name_json = cJSON_GetObjectItem(tensor_json, "name");
	  if (!tensor_arg_name_json) {
	       *error = ln_error_create(LN_ERROR,
					"one of op \"%s\"'s output tensors doesn't have an \"arg_name\" key at tensor index %d",
					name_json->valuestring, i);
	       goto err;
	  }
	  if (!cJSON_IsString(tensor_arg_name_json)) {
	       *error = ln_error_create(LN_ERROR,
					"one of op \"%s\"'s output tensors's arg_name is not a String at tensor index %d",
					name_json->valuestring, i);
	       goto err;
	  }
	  if (!tensor_name_json) {
	       *error = ln_error_create(LN_ERROR,
					"op \"%s\"'s \"%s\" tensor doesn't have a \"name\" key",
					name_json->valuestring,
					tensor_arg_name_json->valuestring);
	       goto err;
	  }
	  if (!cJSON_IsString(tensor_name_json)) {
	       *error = ln_error_create(LN_ERROR,
					"op \"%s\"'s \"%s\" tensor's name is not a String",
					name_json->valuestring,
					tensor_arg_name_json->valuestring);
	       goto err;
	  }
	  tensors_out = ln_tensor_list_append(tensors_out,
                                              tensor_arg_name_json->valuestring,
                                              tensor_name_json->valuestring);
	  i++;
     }
     i = 0;
     cJSON_ArrayForEach(param_json, params_json) {
	  param_arg_name_json = cJSON_GetObjectItem(param_json, "arg_name");
	  if (!param_arg_name_json) {
	       *error = ln_error_create(LN_ERROR,
					"one of op \"%s\"'s params doesn't have an \"arg_name\" key at param index %d",
					name_json->valuestring, i);
	       goto err;
	  }
	  if (!cJSON_IsString(param_arg_name_json)) {
	       *error = ln_error_create(LN_ERROR,
					"one of op \"%s\"'s params's arg_name is not a String at param index %d",
					name_json->valuestring);
	       goto err;
	  }
	  param_value_json = cJSON_GetObjectItem(param_json, "value");
	  if (!param_value_json) {
	       *error = ln_error_create(LN_ERROR,
					"op \"%s\"'s \"%s\" param doesn't have a \"value\" key",
					name_json->valuestring,
					param_arg_name_json->valuestring);
	       goto err;
	  }
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
					  param_arg_name_json, params, error);
	       if (*error)
		    goto err;
	  }
	  else {
	       *error = ln_error_create(LN_ERROR,
					"op \"%s\"'s param \"%s\"'s value is an unsupported JSON type",
					name_json->valuestring,
					param_arg_name_json->valuestring);
	       goto err;
	  }
	  i++;
     }
     proto_op = ln_op_list_find_by_optype(registered_ops,
					  optype_json->valuestring);
     if (!proto_op) {
	  *error = ln_error_create(LN_ERROR,
				   "op \"%s\"'s optype \"%s\" is not registered",
				   name_json->valuestring,
				   optype_json->valuestring);
	  goto err;
     }

     op = ln_op_create_from_proto(proto_op, name_json->valuestring, tensors_in,
                                  tensors_out, params, tensor_table);
     /*
      * op->pre_run() runs here, because we need it to register tensors
      */
     op->pre_run(op->op_arg, error);
     if (*error)
	  goto err_pre_run;

     return op;

err_pre_run:
     ln_op_free(op);
err:
     ln_param_list_free(params);
     ln_tensor_list_free(tensors_in);
     ln_tensor_list_free(tensors_out);
     return NULL;
}

ln_list *ln_parse_ops(const char *json_str, ln_list *registered_ops,
                      ln_hash *tensor_table, ln_error **error)
{
     const cJSON *ops_json;
     const cJSON *op_json;
     cJSON *json;
     ln_list *ops = NULL;
     ln_op *op;

     json = cJSON_Parse(json_str);
     if (!json) {
	  *error = ln_error_create(LN_ERROR, "parsing JSON before: %s",
                                   cJSON_GetErrorPtr());
	  goto err_json;
     }

     ops_json = cJSON_GetObjectItem(json, "ops");
     if (!ops_json) {
	  *error = ln_error_create(LN_ERROR, "top object should have an \"ops\" item");
	  goto err_json;
     }
     if (!cJSON_IsArray(ops_json)) {
	  *error = ln_error_create(LN_ERROR, "item \"ops\" has to be an Array");
	  goto err_json;
     }

     int i = 0;
     cJSON_ArrayForEach(op_json, ops_json) {
	  op = parse_op(op_json, registered_ops, tensor_table, i, error);
	  if (*error) {
	       assert(!op);
	       goto err_op;
	  }
	  ops = ln_list_append(ops, op);
	  i++;
     }

     cJSON_Delete(json);
     return ops;

err_op:
     /*
      * If error occurs in the middle of parsing an op, we should undo
      * everything done by previous ops' successful pre_run()s, by calling
      * their post_run()s, then free the ops and their tensor tables and
      * param tables.
      */
     ln_op_list_do_post_run(ops, error);
     ln_op_list_free_lists_too(ops);
err_json:
     cJSON_Delete(json);
     return NULL;
}

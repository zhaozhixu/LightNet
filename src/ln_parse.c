#include <string.h>
#include <assert.h>
#include "ln_parse.h"
#include "ln_error.h"
#include "ln_util.h"
#include "ln_op.h"
#include "cJSON.h"

static ln_op *parse_op(const cJSON *op_json, ln_list *ops,
		       const ln_list *registered_ops, ln_error **error)
{
     ln_op *op, *proto_op;
     ln_op_arg *op_arg;
     ln_tensor_table *tensors = NULL;
     ln_param_table *params = NULL;

     cJSON *name_json = cJSON_GetObjectItem(op_json, "name");
     cJSON *optype_json = cJSON_GetObjectItem(op_json, "optype");
     cJSON *tensors_json = cJSON_GetObjectItem(op_json, "tensors");
     cJSON *params_json = cJSON_GetObjectItem(op_json, "params");
     if (!name_json) {
	  *error = ln_error_create(LN_ERROR,
				   "an op doesn't have a \"name\" key");
	  return NULL;
     }
     if (!cJSON_IsString(name_json)) {
	  *error = ln_error_create(LN_ERROR, "an op's name is not a String");
	  return NULL;
     }
     if (!optype_json) {
	  *error = ln_error_create(LN_ERROR,
				   "op \"%s\" doesn't have an \"optype\" key",
				   name_json->valuestring);
	  return NULL;
     }
     if (!cJSON_IsString(optype_json)) {
	  *error = ln_error_create(LN_ERROR,
				   "op \"%s\"'s optype is not a String",
				   name_json->valuestring);
	  return NULL;
     }
     if (!tensors_json) {
	  *error = ln_error_create(LN_ERROR,
				   "op \"%s\" doesn't have a \"tensors\" key",
				   name_json->valuestring);
	  return NULL;
     }
     if (!cJSON_IsArray(tensors_json)) {
	  *error = ln_error_create(LN_ERROR,
				   "op \"%s\"'s tensors is not an Array",
				   name_json->valuestring);
	  return NULL;
     }
     if (!params_json) {
	  *error = ln_error_create(LN_ERROR,
				   "op \"%s\" doesn't have a \"params\" key",
				   name_json->valuestring);
	  return NULL;
     }
     if (!cJSON_IsArray(params_json)) {
	  *error = ln_error_create(LN_ERROR,
				   "op \"%s\"'s params is not an Array",
				   name_json->valuestring);
	  return NULL;
     }

     cJSON *tensor_json, *param_json;
     cJSON *tensor_arg_name_json, *tensor_name_json;
     cJSON *param_arg_name_json, *param_value_json;
     tl_tensor *tensor;
     cJSON_ArrayForEach(tensor_json, tensors_json) {
	  tensor_arg_name_json = cJSON_GetObjectItem(tensor_json, "arg_name");
	  tensor_name_json = cJSON_GetObjectItem(tensor_json, "name");
	  if (!tensor_arg_name_json) {
	       *error = ln_error_create(LN_ERROR,
					"one of op \"%s\"'s tensors doesn't have an \"arg_name\" key",
					name_json->valuestring);
	       return NULL;
	  }
	  if (!cJSON_IsString(tensor_arg_name_json)) {
	       *error = ln_error_create(LN_ERROR,
					"one of op \"%s\"'s tensors's arg_name is not a String",
					name_json->valuestring);
	       return NULL;
	  }
	  if (!tensor_name_json) {
	       *error = ln_error_create(LN_ERROR,
					"op \"%s\"'s \"%s\" tensor doesn't have a \"name\" key",
					name_json->valuestring,
					tensor_arg_name_json->valuestring);
	       return NULL;
	  }
	  if (!cJSON_IsString(tensor_name_json)) {
	       *error = ln_error_create(LN_ERROR,
					"op \"%s\"'s \"%s\" tensor's name is not a String",
					name_json->valuestring,
					tensor_arg_name_json->valuestring);
	       return NULL;
	  }
	  tensor = ln_op_list_find_tensor_by_name(ops,
						  tensor_name_json->valuestring);
	  tensors = ln_tensor_table_append(tensors,
					   tensor_name_json->valuestring,
					   tensor_arg_name_json->valuestring,
					   tensor);
     }
     cJSON_ArrayForEach(param_json, params_json) {
	  param_arg_name_json = cJSON_GetObjectItem(param_json, "arg_name");
	  if (!param_arg_name_json) {
	       *error = ln_error_create(LN_ERROR,
					"one of op \"%s\"'s params doesn't have an \"arg_name\" key",
					name_json->valuestring);
	       return NULL;
	  }
	  if (!cJSON_IsString(param_arg_name_json)) {
	       *error = ln_error_create(LN_ERROR,
					"one of op \"%s\"'s params's arg_name is not a String",
					name_json->valuestring);
	       return NULL;
	  }
	  param_value_json = cJSON_GetObjectItem(param_json, "value");
	  if (!param_value_json) {
	       *error = ln_error_create(LN_ERROR,
					"op \"%s\"'s \"%s\" param doesn't have a \"value\" key",
					name_json->valuestring,
					param_arg_name_json->valuestring);
	       return NULL;
	  }
	  if (cJSON_IsNumber(param_value_json)) {
	       params = ln_param_table_append_number(params,
						     param_arg_name_json->valuestring,
						     param_value_json->valuedouble);
	  }
	  else if (cJSON_IsString(param_value_json)) {
	       params = ln_param_table_append_string(params,
						     param_arg_name_json->valuestring,
						     param_value_json->valuestring);
	  }
	  else if (cJSON_IsTrue(param_value_json)) {
	       params = ln_param_table_append_bool(params,
						   param_arg_name_json->valuestring,
						   LN_TRUE);
	  }
	  else if (cJSON_IsFalse(param_value_json)) {
	       params = ln_param_table_append_bool(params,
						   param_arg_name_json->valuestring,
						   LN_FALSE);
	  }
	  else if (cJSON_IsNull(param_value_json)) {
	       params = ln_param_table_append_null(params,
						   param_arg_name_json->valuestring);
	  }
	  else if (cJSON_IsArray(param_value_json)) {

	  }
	  else {
	       *error = ln_error_create(LN_ERROR,
					"op \"%s\"'s param \"%s\"'s value is an unsupported JSON type",
					name_json->valuestring,
					param_arg_name_json->valuestring);
	       goto err;
	  }
     }
     proto_op = ln_op_list_find_by_optype(registered_ops,
					  optype_json->valuestring);
     if (!proto_op) {
	  *error = ln_error_create(LN_ERROR,
				   "op \"%s\"'s optype \"%s\" is not registered"
				   name_json->valuestring,
				   optype_json->valuestring);
	  goto err;
     }

     op_arg = ln_op_arg_create(name_json->valuestring, optype_json->valuestring,
			       tensors, params);
     op = ln_op_create(op_arg, proto_op->pre_run, proto_op->run,
		       proto_op->post_run);
     return op;

err:
     ln_tensor_table_free(tensors);
     ln_param_table_free(params);
     return NULL;
}

ln_list *ln_parse_ops(const char * const json_str,
		      const ln_list *registered_ops, ln_error **error)
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
     cJSON_ArrayForEach(op_json, ops_json) {
	  op = parse_op(op_json, ops, registered_ops, error);
	  if (*error) {
	       assert(!op);
	       goto err_parse_op;
	  }
	  ops = ln_list_append(ops, op);
     }

     cJSON_Delete(json);
     return ops;

err_parse_op:
     ln_op_list_free(ops, TL_TRUE);
err_json:
     cJSON_Delete(json);
     return NULL;
}

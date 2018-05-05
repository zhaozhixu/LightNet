#include <math.h>
#include <assert.h>
#include "ln_op.h"
#include "tl_tensor.h"

static void slice_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *dst_entry, *src_entry;
     ln_param_entry *axis_entry, *start_entry, *len_entry;
     int tensors_n, params_n;
     int axis, start, len;

     /* check tensors and parameters */
     tensors_n = ln_tensor_table_length(op_arg->tensors);
     if (tensors_n != 2) {
	  *error = ln_error_create(LN_ERROR,
				   "%s: slice needs 2 tensors, got %d tensors",
				   op_arg->name, tensors_n);
	  return;
     }
     dst_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "dst");
     if (!dst_entry) {
	  *error = ln_error_create(LN_ERROR,
				   "%s: slice needs a \"dst\" tensor",
				   op_arg->name);
	  return;
     }
     if (dst_entry->tensor) {
	  *error = ln_error_create(LN_WARNING,
				   "%s: slice's \"dst\" tensor \"%s\" is duplicated",
				   op_arg->name, dst_entry->name);
	  return;
     }
     src_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "src");
     if (!src_entry) {
	  *error = ln_error_create(LN_ERROR,
				   "%s: slice needs a \"src\" tensor",
				   op_arg->name);
	  return;
     }
     if (!src_entry->tensor) {
	  *error = ln_error_create(LN_ERROR,
				   "%s: slice's \"src\" tensor \"%s\" is not seen before",
				   op_arg->name, src_entry->name);
	  return;
     }
     params_n = ln_param_table_length(op_arg->params);
     if (params_n != 3) {
	  *error = ln_error_create(LN_ERROR,
				   "%s: slice needs 3 params, got %d params",
				   op_arg->name, params_n);
	  return;
     }
     axis_entry = ln_param_table_find_by_arg_name(op_arg->params, "axis");
     if (!axis_entry) {
	  *error = ln_error_create(LN_ERROR,
				   "%s: slice needs a \"axis\" param",
				   op_arg->name);
	  return;
     }
     start_entry = ln_param_table_find_by_arg_name(op_arg->params, "start");
     if (!start_entry) {
	  *error = ln_error_create(LN_ERROR,
				   "%s: slice needs a \"start\" param",
				   op_arg->name);
	  return;
     }
     len_entry = ln_param_table_find_by_arg_name(op_arg->params, "len");
     if (!len_entry) {
	  *error = ln_error_create(LN_ERROR,
				   "%s: slice needs a \"len\" param",
				   op_arg->name);
	  return;
     }
     axis = (int)axis_entry->value_number;
     start = (int)start_entry->value_number;
     len = (int)len_entry->value_number;
     if (axis < 0 || axis >= src_entry->tensor->ndim) {
	  *error = ln_error_create(LN_ERROR,
				   "%s: slice's \"axis\" param should be \"0 <= axis < src->ndim\"",
				   op_arg->name);
	  return;
     }
     if (start < 0 || start >= src_entry->tensor->dims[axis]) {
	  *error = ln_error_create(LN_ERROR,
				   "%s: slice's \"start\" param should be \"0 <= start < src->dims[axis]\"",
				   op_arg->name);
	  return;
     }
     if (len <= 0 || len > src_entry->tensor->dims[axis]) {
	  *error = ln_error_create(LN_ERROR,
				   "%s: slice's \"len\" param should be \"0 < len <= src->dims[axis]\"",
				   op_arg->name);
	  return;
     }
     if (len + start > src_entry->tensor->dims[axis]) {
	  *error = ln_error_create(LN_ERROR,
				   "%s: slice's params should be \"len + start <= src->dims[axis]\"",
				   op_arg->name);
	  return;
     }

     dst_entry->tensor = tl_tensor_create_slice(src_entry->tensor, axis, len,
						src_entry->tensor->dtype);
}

static void slice_run(ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *dst_entry, *src_entry;
     ln_param_entry *axis_entry, *start_entry, *len_entry;

     dst_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "dst");
     assert(dst_entry);
     src_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "src");
     assert(src_entry);
     axis_entry = ln_param_table_find_by_arg_name(op_arg->params, "axis");
     assert(axis_entry);
     start_entry = ln_param_table_find_by_arg_name(op_arg->params, "start");
     assert(start_entry);
     len_entry = ln_param_table_find_by_arg_name(op_arg->params, "len");
     assert(len_entry);

     tl_tensor_slice(src_entry->tensor, dst_entry->tensor,
		     (int)axis_entry->value_number,
		     (int)start_entry->value_number,
		     (int)len_entry->value_number);
}

static void slice_post_run(ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *dst_entry;

     dst_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "dst");
     assert(dst_entry);
     tl_tensor_free(dst_entry->tensor, TL_TRUE);
}

ln_op ln_op_slice = {
     .name = NULL,
     .type_name = "slice",
     .tensors = NULL,
     .params = NULL,
     .pre_run = slice_pre_run,
     .run = slice_run,
     .post_run = slice_post_run
};

#include <math.h>
#include <assert.h>
#include "ln_op.h"
#include "tl_tensor.h"

/*
 * This function should do the parameter checking and memory allocation.
 */
static void slice_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *dst_entry, *src_entry;
     ln_param_entry *axis_entry, *start_entry, *len_entry;
     int tensors_n, params_n;
     int axis, start, len;

     /* check tensors and parameters */
     tensors_n = ln_tensor_table_length(op_arg->tensors);
     ln_op_check_tensor_num_eq(LN_ERROR, tensors_n, 2);

     src_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "src");
     ln_op_check_tensor_exist(LN_ERROR, src_entry, "src");
     ln_op_check_tensor_defined(LN_ERROR, src_entry);

     dst_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "dst");
     ln_op_check_tensor_exist(LN_ERROR, dst_entry, "dst");
     ln_op_check_tensor_not_defined(LN_WARNING, dst_entry);

     params_n = ln_param_table_length(op_arg->params);
     ln_op_check_param_num_eq(LN_ERROR, params_n, 3);

     axis_entry = ln_param_table_find_by_arg_name(op_arg->params, "axis");
     ln_op_check_param_exist(LN_ERROR, axis_entry, "axis");
     ln_op_check_param_type(LN_ERROR, axis_entry, LN_PARAM_NUMBER);

     start_entry = ln_param_table_find_by_arg_name(op_arg->params, "start");
     ln_op_check_param_exist(LN_ERROR, start_entry, "start");
     ln_op_check_param_type(LN_ERROR, start_entry, LN_PARAM_NUMBER);

     len_entry = ln_param_table_find_by_arg_name(op_arg->params, "len");
     ln_op_check_param_exist(LN_ERROR, len_entry, "len");
     ln_op_check_param_type(LN_ERROR, len_entry, LN_PARAM_NUMBER);

     axis = (int)axis_entry->value_number;
     start = (int)start_entry->value_number;
     len = (int)len_entry->value_number;
     ln_op_check_param_satisfy(LN_ERROR,
			      axis >= 0 && axis < src_entry->tensor->ndim);
     ln_op_check_param_satisfy(LN_ERROR,
			      start >= 0 && start < src_entry->tensor->dims[axis]);
     ln_op_check_param_satisfy(LN_ERROR,
			      len > 0 && len <= src_entry->tensor->dims[axis]);
     ln_op_check_param_satisfy(LN_ERROR,
			      len + start <= src_entry->tensor->dims[axis]);
     /* have checked tensors and parameters */

     /* allocate memory for tensors needing allocation */
     dst_entry->tensor = tl_tensor_create_slice(src_entry->tensor, axis, len,
						src_entry->tensor->dtype);
}

/*
 * Normally we should only do the calculations here. Operations with memory
 * and such should go in pre_run().
 */
static void slice_run(ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *dst_entry, *src_entry;
     ln_param_entry *axis_entry, *start_entry, *len_entry;

     /* Those tensors and params should have been checked in pre_run().
	Further errors should be considered as bugs, so we use asserts here. */
     src_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "src");
     assert(src_entry);
     dst_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "dst");
     assert(dst_entry);
     axis_entry = ln_param_table_find_by_arg_name(op_arg->params, "axis");
     assert(axis_entry);
     start_entry = ln_param_table_find_by_arg_name(op_arg->params, "start");
     assert(start_entry);
     len_entry = ln_param_table_find_by_arg_name(op_arg->params, "len");
     assert(len_entry);

     /* do the real work */
     tl_tensor_slice(src_entry->tensor, dst_entry->tensor,
		     (int)axis_entry->value_number,
		     (int)start_entry->value_number,
		     (int)len_entry->value_number);
}

/*
 * This function should undo everything with memory that pre_run() did.
 */
static void slice_post_run(ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *dst_entry;

     dst_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "dst");
     assert(dst_entry);

     /* free the tensor memory allocated in pre_run() */
     tl_tensor_free_data_too(dst_entry->tensor);
}

static ln_op_arg op_arg_slice = {
     .name = NULL,
     .optype = "slice",
     .tensors = NULL,
     .params = NULL,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_op_slice = {
     .op_arg = &op_arg_slice,
     .pre_run = slice_pre_run,
     .run = slice_run,
     .post_run = slice_post_run
};

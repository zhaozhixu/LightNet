#include <math.h>
#include <assert.h>
#include "ln_op.h"
#include "tl_tensor.h"

static void reshape_pre_run(ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *dst_entry, *src_entry;
     ln_param_entry *dims_entry;
     int tensors_n, params_n;
     int *dims;

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
     ln_op_check_param_num_eq(LN_ERROR, params_n, 1);

     dims_entry = ln_param_table_find_by_arg_name(op_arg->params, "dims");
     ln_op_check_param_exist(LN_ERROR, dims_entry, "dims");

     dims = (int)dims_entry->value_number;
     start = (int)start_entry->value_number;
     len = (int)len_entry->value_number;
     ln_op_check_param_satisfy(LN_ERROR,
			      dims >= 0 && dims < src_entry->tensor->ndim);
     ln_op_check_param_satisfy(LN_ERROR,
			      start >= 0 && start < src_entry->tensor->dims[dims]);
     ln_op_check_param_satisfy(LN_ERROR,
			      len > 0 && len <= src_entry->tensor->dims[dims]);
     ln_op_check_param_satisfy(LN_ERROR,
			      len + start <= src_entry->tensor->dims[dims]);
     /* have checked tensors and parameters */

     /* allocate memory for tensors needing allocation */
     dst_entry->tensor = tl_tensor_create_reshape(src_entry->tensor, dims, len,
						src_entry->tensor->dtype);
}

static void reshape_run(ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *dst_entry, *src_entry;
     ln_param_entry *dims_entry, *start_entry, *len_entry;

     /* Those tensors and params should have been checked in pre_run().
	Further errors should be considered as bugs, so we use asserts here. */
     src_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "src");
     assert(src_entry);
     dst_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "dst");
     assert(dst_entry);
     dims_entry = ln_param_table_find_by_arg_name(op_arg->params, "dims");
     assert(dims_entry);
     start_entry = ln_param_table_find_by_arg_name(op_arg->params, "start");
     assert(start_entry);
     len_entry = ln_param_table_find_by_arg_name(op_arg->params, "len");
     assert(len_entry);

     /* do the real work */
     tl_tensor_reshape(src_entry->tensor, dst_entry->tensor,
		     (int)dims_entry->value_number,
		     (int)start_entry->value_number,
		     (int)len_entry->value_number);
}

static void reshape_post_run(ln_op_arg *op_arg, ln_error **error)
{
     ln_tensor_entry *dst_entry;

     dst_entry = ln_tensor_table_find_by_arg_name(op_arg->tensors, "dst");
     assert(dst_entry);

     /* free the tensor memory allocated in pre_run() */
     tl_tensor_free(dst_entry->tensor, TL_TRUE);
}

static ln_op_arg op_arg_reshape = {
     .name = NULL,
     .optype = "reshape",
     .tensors = NULL,
     .params = NULL,
};

ln_op ln_op_reshape = {
     .op_arg = &op_arg_reshape,
     .pre_run = reshape_pre_run,
     .run = reshape_run,
     .post_run = reshape_post_run
};

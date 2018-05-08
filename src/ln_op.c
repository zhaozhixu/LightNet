#include "ln_op.h"

static ln_op_arg *ln_op_arg_create(const char *name, const char *optype,
			    ln_tensor_table *tensors, ln_param_table *params)
{
     ln_op_arg *op_arg;

     op_arg = ln_alloc(sizeof(ln_op_arg));
     op_arg->name = ln_alloc(sizeof(char)*(strlen(name)+1));
     strcpy(op_arg->name, name);
     op_arg->optype = ln_alloc(sizeof(char)*(strlen(optype)+1));
     strcpy(op_arg->optype, optype);
     op_arg->tensors = tensors;
     op_arg->params = params;

     return op_arg;
}

static void ln_op_arg_free(ln_op_arg *op_arg)
{
     ln_free(op_arg->name);
     ln_free(op_arg->optype);
     ln_free(op_arg);
}

ln_op *ln_op_create(const char *name, const char *optype,
                    ln_tensor_table *tensors, ln_param_table *params,
                    ln_op_func pre_run, ln_op_func run, ln_op_func post_run)
{
     ln_op *op;

     ln_op = ln_alloc(sizeof(ln_op));
     ln_op->op_arg = ln_op_arg_create(name, optype, tensors, params);
     ln_op->pre_run = pre_run;
     ln_op->run = run;
     ln_op->post_run = post_run;

     return op;
}

void ln_op_free(ln_op *op)
{
     ln_op_arg_free(op->op_arg);
     ln_free(op);
}

static void op_free_tables_too_wrapper(void *p)
{
     ln_op *op;

     op = (ln_op *)p;
     ln_tensor_table_free(op->op_arg->tensors);
     ln_param_table_free(op->op_arg->params);
     ln_op_free(p);
}

void ln_op_list_free_tables_too(ln_list *ops)
{
     ln_list_free_deep(ops, op_free_tables_too_wrapper);
}

tl_tensor *ln_op_list_find_tensor_by_name(const ln_list *ops, char *name)
{
     ln_list *l;
     ln_op *op;
     ln_tensor_entry *entry;

     for (l = ops; l; l = l->next) {
	  op = (ln_op *)l->data;
	  entry = ln_tensor_table_find_by_name(op->op_arg->tensors, name);
	  if (entry)
	       break;
     }
     if (!l)
	  return NULL;

     return entry->tensor;
}

static int find_by_optype(void *data1, void *data2)
{
     ln_op *op1, *op2;

     op1 = (ln_op *)data1;
     op2 = (ln_op *)data2;
     return strcmp(op1->op_arg->optype, op2->op_arg->optype);
}

ln_op *ln_op_list_find_by_optype(const ln_list *ops, char *optype)
{
     ln_op cmp_op;
     ln_op *result_op;

     cmp_op.op_arg = ln_op_arg_create("", optype, NULL, NULL);
     result_op = ln_list_find_custom(ops, &cmp_op, find_by_optype);
     ln_op_arg_free(cmp_op.op_arg);

     return result_op;
}

void ln_op_list_do_run(ln_list *ops, ln_error **error)
{
     ln_list *l;
     ln_op *op;

     for (l = ops; l; l = l->next) {
          op = (ln_op *)l->data;
          op->run(op->op_arg, error);
          if (*error)
               return;
     }
}

void ln_op_list_do_post_run(ln_list *ops, ln_error **error)
{
     ln_list *l;
     ln_op *op;

     for (l = ops; l; l = l->next) {
          op = (ln_op *)l->data;
          op->post_run(op->op_arg, error);
          if (*error)
               return;
     }
}

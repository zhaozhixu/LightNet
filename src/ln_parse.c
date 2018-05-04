#include <string.h>
#include "ln_parse.h"
#include "ln_util.h"
#include "cJSON.h"

void ln_parse_ops(const char * const json_str, char **err)
{
     const cJSON *ops;
     const cJSON *op;
     cJSON *ops_json;

     ops_json = cJSON_Parse(json_str);
     if (!ops_json) {
	  const char *err_head = "ERROR: parsing JSON before: ";
	  const char *error_ptr = cJSON_GetErrorPtr();
	  *err = ln_alloc(sizeof(char)*(strlen(err_head)+strlen(error_ptr)+1));
	  strcpy(*err, err_head);
	  strcpy(*err+strlen(err_head), error_ptr);
	  goto END;
     }

     ops = cJSON_GetObjectItem(ops_json, "ops");
     cJSON_ArrayForEach(op, ops) {
	  cJSON *name_json = cJSON_GetObjectItem(op, "name");
	  cJSON *optype_json = cJSON_GetObjectItem(op, "optype");
	  cJSON *tensors_json = cJSON_GetObjectItem(op, "tensors");
	  cJSON *params_json = cJSON_GetObjectItem(op, "params");
     }

END:
     cJSON_Delete(ops_json);
}

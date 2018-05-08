#ifndef _LN_PARAM_H_
#define _LN_PARAM_H_

#include "ln_list.h"

typedef enum ln_param_type ln_param_type;
enum ln_param_type {
     LN_PARAM_INVALID = -1,
     LN_PARAM_NULL = 0,
     LN_PARAM_STRING,
     LN_PARAM_NUMBER,
     LN_PARAM_BOOL,
     LN_PARAM_ARRAY_STRING,
     LN_PARAM_ARRAY_NUMBER,
     LN_PARAM_ARRAY_BOOL
};

typedef struct ln_param_entry ln_param_entry;
struct ln_param_entry {
     char          *arg_name;
     ln_param_type  type;
     int            array_len;
     union {
          char     *value_string;
          double    value_number;
          ln_bool   value_bool;
          char    **value_array_string;
          double   *value_array_number;
          ln_bool  *value_array_bool;
     };
};

typedef ln_list ln_param_table;

#ifdef __cplusplus
extern "C" {
#endif

ln_param_table *ln_param_table_append_string(ln_param_table *table,
                                             const char *arg_name,
                                             const char *string);
ln_param_table *ln_param_table_append_number(ln_param_table *table,
                                             const char *arg_name,
                                             double number);
ln_param_table *ln_param_table_append_bool(ln_param_table *table,
                                           const char *arg_name,
                                           ln_bool bool);
ln_param_table *ln_param_table_append_null(ln_param_table *table,
                                           const char *arg_name);
ln_param_table *ln_param_table_append_array_string(ln_param_table *table,
                                                   const char *arg_name,
                                                   int array_len,
                                                   const char **array_string);
ln_param_table *ln_param_table_append_array_number(ln_param_table *table,
                                                   const char *arg_name,
                                                   int array_len,
                                                   double *array_number);
ln_param_table *ln_param_table_append_array_bool(ln_param_table *table,
                                                 const char *arg_name,
                                                 int array_len,
                                                 ln_bool *array_bool);
void ln_param_table_free(ln_param_table *table);
ln_param_entry *ln_param_table_find_by_arg_name(const ln_param_table *table,
						char *arg_name);
int ln_param_table_length(ln_param_table *table);
const char *ln_param_type_name(ln_param_type type);

#ifdef __cplusplus
}
#endif

#endif	/* _LN_PARAM_H_ */

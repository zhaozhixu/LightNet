#ifndef _LN_PARAM_H_
#define _LN_PARAM_H_

#include "ln_list.h"

typedef enum ln_param_type ln_param_type;
enum ln_param_type {
     LN_PARAM_NUMBER,
     LN_PARAM_STRING,
     LN_PARAM_BOOL,
     LN_PARAM_NULL
};

typedef struct ln_param_entry ln_param_entry;
struct ln_param_entry {
     char          *arg_name;
     char          *value_string;
     double         value_number;
     ln_bool        value_bool;
     ln_param_type  type;
};

typedef ln_list ln_param_table;

#ifdef __cplusplus
extern "C" {
#endif

ln_param_entry *ln_param_entry_create(const char *arg_name, ln_param_type type);
void ln_param_entry_free(ln_param_entry *entry);
void ln_param_entry_set_string(ln_param_entry *entry, const char *string);
void ln_param_entry_set_number(ln_param_entry *entry, double number);
void ln_param_entry_set_bool(ln_param_entry *entry, ln_bool bool);
ln_param_table *ln_param_table_append(ln_param_table *table,
				ln_param_entry *entry);
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
void ln_param_table_free(ln_param_table *table);
ln_param_entry *ln_param_table_find_by_arg_name(const ln_param_table *table,
						char *arg_name);
int ln_param_table_length(ln_param_table *table);

#ifdef __cplusplus
}
#endif

#endif	/* _LN_PARAM_H_ */

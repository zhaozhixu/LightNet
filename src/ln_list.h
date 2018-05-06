#ifndef _LN_LIST_H_
#define _LN_LIST_H_

#include "ln_util.h"

typedef struct ln_list_node ln_list_node;

struct ln_list_node {
     void          *data;
     ln_list_node *next;
};

typedef ln_list_node ln_list;

#ifdef __cplusplus
extern "C" {
#endif

ln_list *ln_list_append(ln_list *list, void *data);
void ln_list_free(ln_list *list);
void ln_list_free_deep(ln_list *list, void (*free_func)(void *));
ln_list *ln_list_nth(ln_list *list, int n);
void *ln_list_nth_data(ln_list *list, int n);
ln_list *ln_list_remove(ln_list *list, void *data);
ln_list *ln_list_remove_nth(ln_list *list, int n);
ln_list *ln_list_insert_nth(ln_list *list, void *data, int n);
ln_list *ln_list_find(const ln_list *list, void *data);
ln_list *ln_list_find_custom(const ln_list *list, void *data, ln_cmp_func cmp);
int ln_list_position(ln_list *list, ln_list *llink);
int ln_list_index(ln_list *list, void *data);
int ln_list_index_custom(ln_list *list, void *data, ln_cmp_func cmp);
int ln_list_length(ln_list *list);
ln_list *ln_list_from_array_size_t(size_t *array, size_t n);
ln_list *ln_list_copy_size_t(ln_list *list);

#ifdef __cplusplus
}
#endif

#endif     /* _LN_LIST_H_ */

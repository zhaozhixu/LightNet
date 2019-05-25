# Data Structures

## Linked List

LightNet uses linked lists to manage a sequence of data structures,
represented by `ln_list`, which is a simple singlely linked list.

    :::c
    struct ln_list {
        void            *data;
        struct ln_list  *next;
    };
    typedef struct ln_list ln_list;

`ln_list` doesn't have an empty header node, every node keeps real data.
It has no "create" functions. Append an element to a `NULL` list to
create a new list. Remember to always use the returned pointer as the newest
list pointer, because the first node may have changed during updating (append,
prepend, remove, insert, etc.).

    :::c
    ln_list *list = NULL;
    list = ln_list_append(list, first_data);

`ln_list` supports the following operations:

- `ln_list *ln_list_prepend(ln_list *list, void *data)`

    Prepend an element at the beginning of the list.
   
- `ln_list *ln_list_append(ln_list *list, void *data)`
    
    Append an element at the end of the list.

- `void ln_list_free(ln_list *list)`

    Free all nodes in the list.

- `void ln_list_free_deep(ln_list *list, void (*free_func)(void *))`

    Free all nodes int the list as well as their elements, which are freed by
    `free_func`.
    
- `ln_list *ln_list_nth(ln_list *list, int n)`

    Return the nth node in a list.

- `void *ln_list_nth_data(ln_list *list, int n)`

    Return the nth element in a list.

- `ln_list *ln_list_remove(ln_list *list, void *data)`

    Remove the first node that has `data` as its element in a list. `data` is 
    compared directly by the pointer value.

- `ln_list *ln_list_remove_nth(ln_list *list, int n)`

    Remove the nth node.

- `ln_list *ln_list_remove_nth_deep(ln_list *list, int n, void (*free_func)(void *))`

    Remove the nth node as well as its element, which is freed by `free_func`.

- `ln_list *ln_list_remove_custom(ln_list *list, void *data, ln_cmp_func cmp)`

    Remove the first node whose element can be compared with `data` by `cmp`
    returning 0.

- `ln_list *ln_list_remove_custom_deep(ln_list *list, void *data, ln_cmp_func cmp, void (*free_func)(void *))`

    Remove the first node as well as its element which can be compared 
    with `data` by `cmp` returning 0. The element is freed by `free_func`.

- `ln_list *ln_list_remove_all_custom_deep(ln_list *list, void *data, ln_cmp_func cmp, void (*free_func)(void *))`

    Remove all the nodes as well as their elements which can be compared 
    with `data` by `cmp` returning 0. The elements are freed by `free_func`.

- `ln_list *ln_list_insert_before(ln_list *list, void *data, ln_list *node)`

    Insert an element `data` before `node` in a list.

- `ln_list *ln_list_insert_nth(ln_list *list, void *data, int n)`

    Insert an element `data` at the nth position in a list.

- `void *ln_list_find(ln_list *list, void *data)`

    Find an element `data` in a list. `data` is compared directly by its 
    pointer value. Return the element if it's found, else return `NULL`.

- `void *ln_list_find_custom(ln_list *list, void *data, ln_cmp_func cmp)`

    Find an element in a list, which can be compared with `data` by `cmp`
    returning 0. Return the element if it's found, else return `NULL`.

- `ln_list *ln_list_find_all_custom(ln_list *list, void *data, ln_cmp_func cmp)`

    Find all elements that can be compared with `data` by `cmp` returning 0.
    Return them in a new list.

- `int ln_list_position(ln_list *list, ln_list *node)`

    Return the position index of `node` in a list.

- `int ln_list_index(ln_list *list, void *data)`

    Return the position index of the first occurence of `data` in a list.
    `data` is compared directly by its pointer value.

- `int ln_list_index_custom(ln_list *list, void *data, ln_cmp_func cmp)`

    Return the position index of the first node whose element can be compared
    with `data` by `cmp` returning 0.

- `int ln_list_length(ln_list *list)`

    Return the length of a list.

- `ln_list *ln_list_reverse(ln_list *list)`

    Reverse the nodes in a list.

- `ln_list *ln_list_from_array_size_t(size_t *array, size_t n)`

    Create a list from an array with length `n`, whose elements are of type
    `size_t`. The elements if the list have the same pointer values as the 
    array's elements' values.

- `ln_list *ln_list_copy(ln_list *list)`

    Create a new list, whose elements' values are the same as `list`'s 
    elements' values.

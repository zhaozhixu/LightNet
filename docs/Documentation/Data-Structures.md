# Data Structures

## Linked List

LightNet uses `ln_list` to manage a sequence of data structures, which is a 
singlely linked list.

    :::c
    struct ln_list {
        void            *data;
        struct ln_list  *next;
    };
    typedef struct ln_list ln_list;

`ln_list` doesn't have an empty header node; every node keeps real data.
It has no "create" functions. To create a new list, append an element to
a `NULL` list. Remember to always use the returned pointer as the newest
list pointer, because the first node may have changed during list alternation
(append, prepend, remove, insert, etc.).

    :::c
    ln_list *list = NULL;
    list = ln_list_append(list, first_data);

To traverse the list in order, use a `for` loop and a temporary variable to 
hold every element of the list, for example:

    :::c
    /* list is a ln_list of element type ln_op */
    ln_list *l;
    ln_op *op;
    for (l = list; l; l = l->next) {
        op = l->data;
        /* operations on op */
    }

`ln_list` provides a macro for the convinence of traversing the list in order.
However, no list alternation shall be done when using this macro:

    :::c
    /* list is a ln_list of element type ln_op */
    ln_op *op;
    LN_LIST_FOREACH(op, list) {
        /* op now hold one element of list */
    }

`ln_list` supports the following operations:

- **`ln_list *ln_list_prepend(ln_list *list, void *data)`**

    Prepend an element at the beginning of the list.
   
- **`ln_list *ln_list_append(ln_list *list, void *data)`**
    
    Append an element at the end of the list.

- **`void ln_list_free(ln_list *list)`**

    Free all nodes in the list.

- **`void ln_list_free_deep(ln_list *list, void (*free_func)(void *))`**

    Free all nodes int the list as well as their elements, which are freed by
    `free_func`.
    
- **`ln_list *ln_list_nth(ln_list *list, int n)`**

    Return the nth node in the list.

- **`void *ln_list_nth_data(ln_list *list, int n)`**

    Return the nth element in the list.

- **`ln_list *ln_list_remove(ln_list *list, void *data)`**

    Remove the first node that has `data` as its element in the list. `data` is 
    compared directly by the pointer value.

- **`ln_list *ln_list_remove_nth(ln_list *list, int n)`**

    Remove the nth node.

- **`ln_list *ln_list_remove_nth_deep(ln_list *list, int n, void (*free_func)(void *))`**

    Remove the nth node as well as its element, which is freed by `free_func`.

- **`ln_list *ln_list_remove_custom(ln_list *list, void *data, ln_cmp_func cmp)`**

    Remove the first node whose element can be compared with `data` by `cmp`
    returning 0.

- **`ln_list *ln_list_remove_custom_deep(ln_list *list, void *data, ln_cmp_func cmp, void (*free_func)(void *))`**

    Remove the first node as well as its element which can be compared 
    with `data` by `cmp` returning 0. The element is freed by `free_func`.

- **`ln_list *ln_list_remove_all_custom_deep(ln_list *list, void *data, ln_cmp_func cmp, void (*free_func)(void *))`**

    Remove all the nodes as well as their elements which can be compared 
    with `data` by `cmp` returning 0. The elements are freed by `free_func`.

- **`ln_list *ln_list_insert_before(ln_list *list, void *data, ln_list *node)`**

    Insert an element `data` before `node` in the list.

- **`ln_list *ln_list_insert_nth(ln_list *list, void *data, int n)`**

    Insert an element `data` at the nth position in the list.

- **`void *ln_list_find(ln_list *list, void *data)`**

    Find an element `data` in the list. `data` is compared directly by its 
    pointer value. Return the element if it's found, else return `NULL`.

- **`void *ln_list_find_custom(ln_list *list, void *data, ln_cmp_func cmp)`**

    Find an element in the list, which can be compared with `data` by `cmp`
    returning 0. Return the element if it's found, else return `NULL`.

- **`ln_list *ln_list_find_all_custom(ln_list *list, void *data, ln_cmp_func cmp)`**

    Find all elements that can be compared with `data` by `cmp` returning 0.
    Return them in a new list.

- **`int ln_list_position(ln_list *list, ln_list *node)`**

    Return the position index of `node` in the list.

- **`int ln_list_index(ln_list *list, void *data)`**

    Return the position index of the first occurence of `data` in the list.
    `data` is compared directly by its pointer value.

- **`int ln_list_index_custom(ln_list *list, void *data, ln_cmp_func cmp)`**

    Return the position index of the first node whose element can be compared
    with `data` by `cmp` returning 0.

- **`int ln_list_length(ln_list *list)`**

    Return the length of the list.

- **`ln_list *ln_list_reverse(ln_list *list)`**

    Reverse the nodes in the list.

- **`ln_list *ln_list_from_array_size_t(size_t *array, size_t n)`**

    Create the list from an array with length `n`, whose elements are of type
    `size_t`. The elements if the list have the same pointer values as the 
    array's elements' values.

- **`ln_list *ln_list_copy(ln_list *list)`**

    Create a new list, whose elements' values are the same as `list`'s 
    elements' values.

## Queue

`ln_queue` is a simple single-in single-out linked list, using two list nodes
for its head and tail.

    :::c
    struct ln_queue {
        ln_list  *head;
        ln_list  *tail;
        size_t    size;
    };
    typedef struct ln_queue ln_queue;

`ln_queue` supports the following operations:

- **`ln_queue *ln_queue_create(void)`**

    Create an empty queue.

- **`ln_queue *ln_queue_enqueue(ln_queue *queue, void *data)`**

    Add an element to the head of the queue. Create a new queue if 
    `queue == NULL`.

- **`void *ln_queue_dequeue(ln_queue *queue)`**

    Remove and return the element at the tail of the queue.

- **`void ln_queue_free(ln_queue *queue)`**

    Free the queue.

## Hash Table

LightNet uses `ln_hash` to construct key-value pairs that can be used for quick
lookups. `ln_hash` is an opaque structure in other modules. It uses separate
chaining method to store the key-value pairs, and its implementation references
the [HashMap](http://hg.openjdk.java.net/jdk7/jdk7/jdk/file/9b8c96f96a0f/src/share/classes/java/util/HashMap.java) in OpenJDK 7.

`ln_hash` supports the following operations:

- **`ln_hash *ln_hash_create_full(ln_hash_func hash_func, ln_cmp_func cmp_func, ln_free_func free_k_func, ln_free_func free_v_func, int init_capacity, float load_factor)`**

    Create a hash table with full arguments.
    
    `hash_func` is the hash function of the key's data type.
    
    `cmp_func` is the comparison function of the key's data type.
    
    `free_k_func` is the free function of the key's data type. If it's `NULL`,
    `ln_hash` will not free the key when it is removed.
    
    `free_v_func` is the free function of the value's data type. If it's `NULL`,
    `ln_hash` will not free the value when it is removed.
    
    `init_capacity` is the initial capacity of the table.
    
    `load_factor` is the load factor of the table. When the ratio of the number
    of elements to the capacity exceeds this factor, the table will expand its
    capacity.

- **`ln_hash *ln_hash_create(ln_hash_func hash_func, ln_cmp_func cmp_func, ln_free_func free_k_func, ln_free_func free_v_func)`**

    Create a hash table with some default arguments.
    
    `hash_func` is the hash function of the key's data type.
    
    `cmp_func` is the comparison function of the key's data type.
    
    `free_k_func` is the free function of the key's data type. If it's `NULL`,
    `ln_hash` will not free the key when it is removed.
    
    `free_v_func` is the free function of the value's data type. If it's `NULL`,
    `ln_hash` will not free the value when it is removed.

- **`void ln_hash_free(ln_hash *hash)`**

    Free a hash table. Also free the keys and/or values if either of the free
    functions is provided when creating this table.

- **`void ln_hash_init(ln_hash *hash, ln_hash_init_entry *init_array)`**

    Initialize an empty hash table with the key-value pairs in `init_array`. 
    The elements of `init_array` are defined as:
    
        :::c
        struct ln_hash_init_entry {
            void *key;
            void *value;
        };
        typedef struct ln_hash_init_entry ln_hash_init_entry;
        
    This array should be ended with a `{NULL, NULL}` entry.

- **`int ln_hash_insert(ln_hash *hash, const void *key, void *value)`**

    Insert a key-value pair to the hash table. Return 0 if the key already
    exists and the old value is replaced by `value` (it will be freed if its
    free function is provided when creating the table), else return 1.

- **`void *ln_hash_find(ln_hash *hash, const void *key)`**

    Find a value in the hash table with `key`. If it is found, return the value,
    else return `NULL`. In case of existing legal `NULL` values, use 
    `ln_hash_find_extended`.

- **`int ln_hash_find_extended(ln_hash *hash, const void *key, void **found_key, void **found_value)`**

    Find a value in the hash table with `key`. If it is found, return 1, else
    return 0. The found key and value will be stored in `found_key` and 
    `found_value` respectively. If either of `found_key` or `found_value` is
    `NULL`, the found key or value will be ignored.

- **`int ln_hash_remove(ln_hash *hash, const void *key)`**

    Remove a key-value pair in the hash table. The key and/or value will also be
    freed if either of the free functions is provided when creating the table.

- **`int ln_hash_size(ln_hash *hash)`**

    Return the number of key-value pairs in the hash table.

## Graph




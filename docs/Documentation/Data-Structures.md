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

LightNet use `ln_graph` to represent computing graphs. `ln_graph` uses ajiacency
list method to implement directed graphs, which uses a linked list to store the
graph nodes; every node has its in-degree, out-degree,  a list of its previous
edges and nodes, and a list of its next edges and nodes.

    :::c
    struct ln_graph_node {
        void        *data;
        ln_list     *out_edge_nodes;  	/* data type is ln_graph_edge_node */
        ln_list     *in_edge_nodes;
        ln_cmp_func  node_data_cmp;
        size_t       indegree;
        size_t       outdegree;
    };
    typedef struct ln_graph_node ln_graph_node;

    struct ln_graph_edge_node {         /* store the ajiacency edge and node */
        void           *edge_data;
        ln_graph_node  *node;
        ln_cmp_func     edge_data_cmp;
    };
    typedef struct ln_graph_edge_node ln_graph_edge_node;
    
    struct ln_graph {
        size_t       size;
        ln_list     *nodes;         	/* data type is ln_graph_node */
        ln_cmp_func  node_data_cmp;
        ln_cmp_func  edge_data_cmp;
    };
    typedef struct ln_graph ln_graph;

`ln_graph` supports the following operations:

- **`ln_graph_node *ln_graph_node_create(void *data, ln_cmp_func node_data_cmp)`**

    Create a graph node with node data and the comparison function of node data.

- **`void ln_graph_node_free(ln_graph_node *node)`**

    Free a graph node.

- **`ln_graph_edge_node *ln_graph_edge_node_create(void *edge_data, ln_graph_node *node, ln_cmp_func edge_data_cmp)`**

    Create a graph edge node with edge data, the graph node, and the comparison
    function of edge data.

- **`void ln_graph_edge_node_free(ln_graph_edge_node *edge_node)`**

    Free a graph edge node.

- **`ln_graph *ln_graph_create(ln_cmp_func node_cmp, ln_cmp_func edge_cmp)`**

    Create an empty graph with the comparison functions of node data and edge
    data.

- **`void ln_graph_free(ln_graph *graph)`**

    Free a graph.

- **`ln_graph_node *ln_graph_add(ln_graph *graph, void *data)`**

    Create and add a node to the graph with `data` as node data. Return the 
    created node.

- **`ln_graph_node *ln_graph_find(ln_graph *graph, void *data)`**

    Find the first graph node that has the same data as `data`. Return the found
    node if the node is found, else return `NULL`.

- **`void ln_graph_link(ln_graph *graph, void *data1, void *data2, void *edge_data)`**

    Link the first node that has the same data as `data1` to the first node 
    that has the same data as `data2`, with a newly created edge of `edge_data`.

- **`void ln_graph_link_node(ln_graph *graph, ln_graph_node *node1, ln_graph_node *node2, void *edge_data)`**

    Link `node1` to `node2` with a newly created edge of `edge_data`.

- **`void *ln_graph_unlink(ln_graph *graph, void *data1, void *data2, void *edge_data)`**

    Unlink the first node that has the same data as `data1` and the first node 
    that has the same data as `data2`, with a edge of `edge_data`. If 
    `edge_data == NULL`, only compare `data1` and `data2`.

- **`void *ln_graph_unlink_node(ln_graph_node *node1, ln_graph_node *node2, void *edge_data)`**

    Unlink `node1` and `node2` with a edge of `edge_data`. If
    `edge_data == NULL`, only compare `data1` and `data2`.

- **`ln_graph *ln_graph_copy(ln_graph *graph)`**

    Copy a graph.

- **`int ln_graph_num_outlier(ln_graph *graph)`**

    Return the number of nodes that don't link with any other nodes.

- **`int ln_graph_topsort(ln_graph *graph, ln_list **layers)`**

    Run topological sort on the graph. Sorted nodes are returned in `layers`,
    which is a pointer to a list of layers. All nodes in the same layer has the
    topological order. 
    Return -1 if the graph has a cycle, else return the number of layers.
    
- **`void ln_graph_free_topsortlist(ln_list *layers)`**

    Free the layers return by `ln_graph_topsort`.

- **`void ln_graph_fprint(FILE *fp, ln_graph *graph, ln_fprint_func print_node, ln_fprint_func print_edge)`**

    Print the graph to stream `fp`. Graph node data are printed by `print_node`.
    Graph edge data are printed by `print_edge`.
    
## Memory Model

Every computing device in heterogeneous platforms may have its own type of
memory. LightNet uses an enumeration type `ln_mem_type` to describe them.
Every `ln_mem_type` should have an instance of `ln_mem_info` in `ln_mem.c`.

    :::c
    enum ln_mem_type {
        LN_MEM_NONE = 0,
        LN_MEM_CPU,
    #ifdef LN_CUDA
        LN_MEM_CUDA,
    #endif
    /* ... may be other mem types here */
        LN_MEM_TYPE_SIZE
    };
    typedef enum ln_mem_type ln_mem_type;

    struct ln_mem_info {
        const char  *name;
        void      *(*alloc_func)(size_t n);
        void       (*free_func)(void *p);
        void      *(*memset_func)(void *s, int c, size_t n);
        size_t       max_size;
        size_t       align_size;
    };
    typedef struct ln_mem_info ln_mem_info;

Every `ln_mem_type` has its own information, in which `alloc_func`, 
`free_func`, `memset_func` are its memory operations, as `malloc`, `free`,
`memset` in the standard C library. `max_size` is the maximum bytes the memory
type can store. `align_size` is the alignment bytes the memory type requires.

`ln_mem_type` supports the following operations:

- **`const char *ln_mem_type_name(ln_mem_type mtype)`**

    Return the name of `mtype`.

- **`const ln_mem_info ln_mem_type_info(ln_mem_type mtype)`**

    Return the `ln_mem_info` of `mtype`.

- **`ln_copy_func ln_mem_type_copy_func(ln_mem_type dst_mtype, ln_mem_type src_mtype)`**

    Return the copy function from `src_mtype` to `dst_mtype`, as `memcpy` in the
    standard C library.

Besides, LightNet uses virtual memory pools to make overall arrangements for
memory allocations and releases. This step pre-plans the memory offsets of 
tensors, which can be converted to real memory addresses in run time.
`ln_mem_pool` is the virtual memory pool structure.

`ln_mem_pool` supports the following operations:

- **`ln_mem_pool *ln_mem_pool_create(size_t size, size_t align_size)`**
  
    Create a virtual memory pool.

- **`void ln_mem_pool_free(ln_mem_pool *mem_pool)`**

    Free a virtual memory pool.

- **`size_t ln_mem_pool_alloc(ln_mem_pool *mem_pool, size_t size)`**

    Return the allocated memory offset (starting from 0) from the memory pool.

- **`void ln_mem_pool_dealloc(ln_mem_pool *mem_pool, size_t addr)`**

    Deallocate a memory offset.

- **`int ln_mem_pool_exist(ln_mem_pool *mem_pool, size_t addr)`**

    Test if a memory offset is allocated before.

- **`void ln_mem_pool_dump(ln_mem_pool *mem_pool, FILE *fp)`**

    Dump the memory layout of the memory pool.

- **`ln_hash *ln_mem_pool_table_create(void)`**

    Create a hash table of all memory pools. 
    The table takes `ln_mem_type` as keys and `ln_mem_pool` as values.

- **`void ln_mem_pool_table_free(ln_hash *mpt)`**

    Free the memory pool hash table returned by `ln_mem_pool_table_create`.

## Tensors



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

    :::c
    /* list is a ln_list of element type ln_op */
    ln_op *op;
    LN_LIST_FOREACH(op, list) {
        /* op now hold one element of list */
    }

!!!note
    No list alternation shall be done when using `LN_LIST_FOREACH`.

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
    exists and the old value will be replaced by `value` (it will be freed if
    its free function is provided when creating the table), else return 1.

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
the memory allocations and releases of all kinds of memory types.
This step pre-plans the memory offsets of tensors, which can be converted to 
real memory addresses in run time.
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

## Tensor

LightNet uses `ln_tensor` as the basic data storage structure.
`ln_tensor` is used as operator's input and output, such as a
convolution's input image, output feature map, and weights.
`ln_tensor` further uses [TensorLight](https://github.com/zhaozhixu/TensorLight)
as its basic tensor operation library, which is seperated from LightNet on
purpose so that it can be used widely beyond LightNet.

TensorLight implements many universal tensor operations. There are some simple
operations such as slice, transpose, concatation, relu, as well as some 
complicated operations such as bounding boxes' coordinate transformation,
YOLO's object detection. Developers are welcomed to add their own useful
operations to TensorLight, which can be used not only in LightNet, but also 
in other projects.

In both projects, a tensor refers to a `tl_tensor` structure, which has the
tensor's meta infomation and its data pointer.

    :::c
    struct tl_tensor {
        tl_dtype          dtype;          /* data type */
        int               len;            /* number of elements */
        int               ndim;           /* number of dimensions */
        int              *dims;           /* array of dimensions */
        void             *data;           /* data pointer */
        struct tl_tensor *owner;          /* data owner, NULL if it's itself */
        void             *backend_data;   /* for other backend dependent data */
    };
    typedef struct tl_tensor tl_tensor;
    
A tensor's data type is represented by a enumeration type `tl_dtype`. It 
supports many common data types such as double, float, 32/16/8 bit 
signed/unsigned integers, and bool.

    :::c
    enum tl_dtype {
        TL_DTYPE_INVALID = -1,
        TL_DOUBLE = 0,
        TL_FLOAT,
        TL_INT32,
        TL_INT16,
        TL_INT8,
        TL_UINT32,
        TL_UINT16,
        TL_UINT8,
        TL_BOOL,
        TL_DTYPE_SIZE
    };
    typedef enum tl_dtype tl_dtype;
    
`tl_tensor` supports many tensor operations, see [TensorLight](https://github.com/zhaozhixu/TensorLight) for more details.

In LightNet, all tensors in a NN model are managed by a tensor table. The tensor
table is a hash table, with tensors' names as its keys and tensor table entries
as its values. The tensor table entry structure is defined as follows.

    :::c
    /* NOTE: ALWAYS access tensor entry via its name in tensor table, since the
       entry may be not the same during passes. It is owned by the tensor table. */
    struct ln_tensor_entry {
        char        *name;          /* tensor name */
        tl_tensor   *tensor;        /* pointer to the underlying tl_tensor */
        char        *owner;         /* owner tensor's name of the tensor's data */
        char        *creater;       /* operator name who creates the tensor */
        size_t       offset;        /* offset address of the tensor's data */
        int          isstatic;      /* the tensor is static or not */
        ln_mem_type  mtype;         /* memory type */
    };
    typedef struct ln_tensor_entry ln_tensor_entry;

Every tensor must be created by an operator, and `creater` records that
operator's name. Considering the computing efficiency, some tensors may share
the same underlying data pointer, in which case `owner` records the name of the
tensor who actually owns the data. `offset` is the relative address assigned to
the tensor in memory planning process, and it is initially 0, which is invalid 
at run time. Some tensors' memory may not be freed after allocation, in which
case `isstatic` should be labeled as 1 to indicate that it's static. Finally,
`mtype` is the memory type of the tensor's data.

`ln_tensor_entry` supports the following operations:

- **`ln_tensor_entry *ln_tensor_entry_create(const char *name, tl_tensor *tensor)`**

    Create a tensor entry with `name` and the underlying `tensor`.
    
- **`void ln_tensor_entry_free(ln_tensor_entry *entry)`**

    Free a tensor entry.

- **`void ln_tensor_entry_free_tensor_too(ln_tensor_entry *entry)`**

    Free the tensor entry as well as the underlying `tensor`,
    but not free the raw data pointer `tensor->data`.

- **`void ln_tensor_entry_set_owner(ln_tensor_entry *entry, ln_hash *tensor_table, char *owner)`**

    Set the owner tensor name to `owner` with which the tensor entry
    shares its data.

- **`void ln_tensor_entry_set_creater(ln_tensor_entry *entry, const char *creater)`**

    Set the name of the creater operator of this tensor entry to `creater`.

The tensor table supports the following operations:

- **`ln_hash *ln_tensor_table_create(void)`**
  
    Create a tensor table.

- **`int ln_tensor_table_insert(ln_hash *table, ln_tensor_entry *entry)`**

    Insert a new tensor entry to the tensor table. The behaviour is consistent
    with `ln_hash_insert()`.

- **`int ln_tensor_table_remove(ln_hash *table, const char *name)`**

    Remove a tensor entry `name` from the tensor table.

- **`ln_tensor_entry *ln_tensor_table_find(ln_hash *table, const char *name)`**

    Find a tensor entry `name` in the tensor table.

- **`void ln_tensor_table_free(ln_hash *table);`**

    Free the tensor table as well as all its tensor entries.

- **`void ln_tensor_table_set_data(ln_hash *table, const char *name, const void *data)`**

    Copy `data` to the underlying memory region of tensor entry `name`.

- **`void *ln_tensor_table_get_data(ln_hash *table, const char *name, void *data)`**

    Copy the underlying memory region of tensor entry `name` to `data`.

- **`size_t ln_tensor_table_data_size(ln_hash *table, const char *name)`**

    Get the data size of the tensor entry `name` in bytes.

- **`void ln_tensor_table_load_trt_weight_file(ln_hash *table, const char *file)`**

    Copy the weights from `file` to the tensor entries accordingly. `file`
    should follow the weight format of TensorRT, which can be showned with
    `tools/genwts.pl -h`.

When removing a tensor or inserting a different tensor with the same name 
as another tensor, the tensor table will free the old table entry and its 
`tensor` field, but not free `tensor->data`. So we should always insert tensors 
with `NULL` data.

Besides tensor table entry, this mudule also defines a tensor list entry
structure `ln_tensor_list_entry`.

    :::c
    struct ln_tensor_list_entry {
        char            *name;
        char            *arg_name;
        size_t           offset;
    };
    typedef struct ln_tensor_list_entry ln_tensor_list_entry;
    
`ln_tensor_list_entry` is used in an operator's input/output tensor list. `name`
and `offset` are the same as those in the tensor's table entry. `name` is used
to find the tensor table entry from the operator. `offset` is used to store
the memory-planned address in the optimized output operator stream. `arg_name`
is the argument name of the tensor in the operator, such as "stride" in an 
"conv2d" operator.

`ln_tensor_list_entry` and tensor list supports the following operations:

- **`ln_tensor_list_entry *ln_tensor_list_entry_create(const char *arg_name, const char *name)`**

    Create a tensor list entry.

- **`void ln_tensor_list_entry_free(ln_tensor_list_entry *entry)`**

    Free a tensor list entry.

- **`ln_list *ln_tensor_list_append(ln_list *list, const char *arg_name, const char *name)`**
  
  Append to a tensor list with an entry that has `arg_name` and `name`.

- **`void ln_tensor_list_free(ln_list *list)`**

    Free a tensor list as well as its entries.

- **`ln_list *ln_tensor_list_copy(ln_list *list)`**

    Copy a tensor list. Entries are newly created for the new list.

- **`char *ln_tensor_list_find_name(ln_list *list, const char *arg_name)`**

    Return the tensor list entry's name that has `arg_name` from the tensor list.

- **`ln_tensor_list_entry *ln_tensor_list_find_by_arg_name(ln_list *list, const char *arg_name)`**

    Find the tensor list entry that has `arg_name` from the tensor list.

- **`ln_tensor_list_entry *ln_tensor_list_find_by_name(ln_list *list, const char *name)`**

    Find the tensor list entry that has `name` from the tensor list.

- **`ln_tensor_entry *ln_tensor_list_find_entry(ln_list *list, ln_hash *tensor_table, const char *arg_name)`**

    Find the tensor entry from `tensor_table` that has the same name with the 
    tensor list entry from `list` that has `arg_name` as its arg name.

- **`int ln_tensor_list_length(ln_list *list)`**

    Return the tensor list length.

- **`int ln_tensor_list_unique_arg_name(ln_list *list, char *buf, const char *prefix)`**

    Create an arg name that is unique in the tensor list.
    The arg name is prefixed with `prefix` and subfixed with a serial number,
    so that unique names will be created for a same prefix.
    The arg name' length should be less than `LN_MAX_NAME_LEN`. If its length
    exceeds that limit, an internal error will be emited and the program will 
    abort. The arg name  will be printed in `buf`. The return value is the 
    subfixed number.
    
## Parameter

Like `ln_tensor_list_entry`, LightNet also uses `ln_param_entry` to represent 
parameters used in an operator's parameter list. Different from `ln_tensor`,
`ln_param_entry` is often used as the configuration for the operator, but
not the input or output of the operator in the semantic meaning, such as the
stride, padding or dilation configuration for a convolution operator.

    :::c
    enum ln_param_type {
        /* NULL should always be the first type */
        LN_PARAM_NULL = 0,
        LN_PARAM_STRING,
        LN_PARAM_NUMBER,
        LN_PARAM_BOOL,
        LN_PARAM_ARRAY_STRING,
        LN_PARAM_ARRAY_NUMBER,
        LN_PARAM_ARRAY_BOOL,
        LN_PARAM_INVALID
        /* INVALID should always be the last type */
    };
    typedef enum ln_param_type ln_param_type;

    struct ln_param_entry {
        char          *arg_name;           /* argument name in tensor list */
        ln_param_type  type;               /* parameter type */
        int            array_len;          /* array length if it's an array */
        double         value_double;       /* double value, as belows... */
        float          value_float;
        int            value_int;
        ln_bool        value_bool;
        char          *value_string;
        char         **value_array_string;
        double        *value_array_double;
        float         *value_array_float;
        int           *value_array_int;
        ln_bool       *value_array_bool;
    };
    typedef struct ln_param_entry ln_param_entry;

The enum type `ln_param_type` represents the param value's data type.
There are three special case here:

1. If the param type is `LN_PARAM_NULL`, the param entry's `value_...` fields
are ignored and the param value is treated as `null`, which could represent some
special uncommon values. 
2. If the param type is `LN_PARAM_NUMBER`, the param entry's `value_double`,
`value_float`, `value_int` are set in the same time when setting the param's
value with `ln_param_set_satu_...` functions, and when the number is 
beyond the data type's representable region, the maximum or minimum value will
be set as the data type's value.
3. If the param type is `LN_PARAM_ARRAY_NUMBER`, the param entry's 
`value_array_double`, `value_array_float`, `value_array_int` are set in the same
time when setting the param's value with `ln_param_set_satu_...` functions, 
and when an array's element is beyond the data type's representable 
region, the maximum or minimum value will be set as the number array's element.

`ln_param_entry` supports the following operations:

- **`ln_param_entry *ln_param_entry_create(const char *arg_name, ln_param_type type)`**

    Create a param entry.

- **`void ln_param_entry_free(ln_param_entry *entry)`**

    Free a param entry.
    
- **`const char *ln_param_type_name(ln_param_type type)`**

    Return the param type's string representation.
    
- **`ln_param_entry *ln_param_entry_copy(const ln_param_entry *pe)`**

    Copy the param entry.

- **`void ln_param_vset(ln_param_entry *entry, va_list ap)`**

    Used to write param setting functions with variable-length parameters.

- **`void ln_param_set(ln_param_entry *entry, ...)`**

    Set the param entry's value. The argument after `entry` will be interpreted
    as the appropriate data type according to `entry->type`, and set as the 
    entry value.

- **`void ln_param_set_null(ln_param_entry *entry)`**

    Set the param entry's type as `LN_PARAM_NULL`.

- **`void ln_param_set_bool(ln_param_entry *entry, ln_bool bool_value)`**

    Set the param entry's bool value.

- **`void ln_param_set_string(ln_param_entry *entry, const char *string)`**

    Set the param entry's string value.

- **`void ln_param_set_satu_number(ln_param_entry *entry, double number)`**

    Set the param entry's number value. The entry's `value_double`, 
    `value_float`, `value_int` fields are set in the same time.
    `value_float`, `value_int` are set in a saturation manner.

- **`void ln_param_set_satu_array_number(ln_param_entry *entry, int array_len, const double *array_number)`**

    Set the param entry's number array value. The entry's `value_array_double`, 
    `value_array_float`, `value_array_int` fields are set in the same time.
    `value_array_float`, `value_array_int` are set in a saturation manner.

- **`void ln_param_set_satu_array_double(ln_param_entry *entry, int array_len, const double *array_number)`**

    Set the param entry's number array value. The entry's `value_array_double`, 
    `value_array_float`, `value_array_int` fields are set in the same time.
    `value_array_float`, `value_array_int` are set in a saturation manner.

- **`void ln_param_set_satu_array_float(ln_param_entry *entry, int array_len, const float *array_number)`**

    Set the param entry's number array value. The entry's `value_array_double`, 
    `value_array_float`, `value_array_int` fields are set in the same time.
    `value_array_int` is set in a saturation manner.

- **`void ln_param_set_satu_array_int(ln_param_entry *entry, int array_len, const int *array_number)`**

    Set the param entry's number array value. The entry's `value_array_double`, 
    `value_array_float`, `value_array_int` fields are set in the same time.

- **`void ln_param_set_array_string(ln_param_entry *entry, int array_len, const char **array_string)`**

    Set the param entry's string array value.

- **`void ln_param_set_array_bool(ln_param_entry *entry, int array_len, const ln_bool *array_bool)`**

    Set the param entry's bool array value.
    
The param list supports the following operations:

- **`ln_list *ln_param_list_append_empty(ln_list *list, const char *arg_name, ln_param_type ptype)`**

    Append an empty param entry to the param list. Entry values are initialized
    to zeros.

- **`ln_list *ln_param_list_append_string(ln_list *list, const char *arg_name, const char *string)`**

    Append a string param entry to the param list.

- **`ln_list *ln_param_list_append_number(ln_list *list, const char *arg_name, double number)`**

    Append a number param entry to the param list. Numbers are set in a 
    saturation manner.

- **`ln_list *ln_param_list_append_double(ln_list *list, const char *arg_name, double number)`**

    Append a double param entry to the param list. Numbers are set in a 
    saturation manner.

- **`ln_list *ln_param_list_append_float(ln_list *list, const char *arg_name, float number)`**

    Append a float param entry to the param list. Numbers are set in a 
    saturation manner.

- **`ln_list *ln_param_list_append_int(ln_list *list, const char *arg_name, int number)`**

    Append an int param entry to the param list.

- **`ln_list *ln_param_list_append_bool(ln_list *list, const char *arg_name, ln_bool bool_value)`**

    Append a bool param entry to the param list.

- **`ln_list *ln_param_list_append_null(ln_list *list, const char *arg_name)`**

    Append a null param entry to the param list.

- **`ln_list *ln_param_list_append_array_string(ln_list *list, const char *arg_name, int array_len, const char **array_string)`**

    Append a string array param entry to the param list.

- **`ln_list *ln_param_list_append_array_number(ln_list *list, const char *arg_name, int array_len, const double *array_number)`**

    Append a number array param entry to the param list. Numbers are set in a 
    saturation manner.

- **`ln_list *ln_param_list_append_array_double(ln_list *list, const char *arg_name, int array_len, const double *array_number)`**

    Append a double array param entry to the param list. Numbers are set in a 
    saturation manner.

- **`ln_list *ln_param_list_append_array_float(ln_list *list, const char *arg_name, int array_len, const float *array_number)`**

    Append a float array param entry to the param list. Numbers are set in a 
    saturation manner.

- **`ln_list *ln_param_list_append_array_int(ln_list *list, const char *arg_name, int array_len, const int *array_int)`**

    Append an int array param entry to the param list.

- **`ln_list *ln_param_list_append_array_bool(ln_list *list, const char *arg_name, int array_len, const ln_bool *array_bool)`**

    Append a bool array param entry to the param list.

- **`void ln_param_list_free(ln_list *list)`**

    Free a param list, as well as the array values (if array values are set).

- **`ln_list *ln_param_list_copy(ln_list *list)`**

    Copy a param list.

- **`ln_param_entry *ln_param_list_find(ln_list *list, const char *arg_name)`**

    Find a param entry with its arg name.

- **`ln_param_entry *ln_param_list_find2(ln_list *list, const char *arg_name1, const char *arg_name2)`**

    Find a param entry with its arg name. The arg name is assembled as
    `{arg_name1}_{arg_name2}`.

- **`int ln_param_list_length(ln_list *list)`**

    Return the param list's length.

- **`int ln_param_list_unique_arg_name(ln_list *list, char *buf, const char *prefix)`**

    Create an arg name that is unique in the param list.
    The arg name is prefixed with `prefix` and subfixed with a serial number,
    so that unique names will be created for a same prefix.
    The arg name' length should be less than `LN_MAX_NAME_LEN`. If its length
    exceeds that limit, an internal error will be emited and the program will 
    abort. The arg name  will be printed in `buf`. The return value is the 
    subfixed number.

## Operator

LightNet use `ln_op` to represent an operator, which receives some input tensors
and compute the output tensors according to its parameters.
A `ln_op` has its own life cycle, as showned below.

<img style="transform:scale(0.7)" src=../img/op_state.png>

`ln_op` has 4 states in its lift cycle: **init**, **checked**, **ready**, **end**.
After created, a `ln_op` is in the **init** state. It has to execute its `pre_run`
function to check the validity of its input tensors and parameters, define 
its output tensors' shape, data type and so on, register its output tensors in the
tensor table, and then enter the **checked** state. 
For most operators, **checked** state is equivalent to **ready** state,
while there are some operators that need to execute its `static_run` function 
first to initialize their private data. 
In the **ready** state, an operator can execute its `run` function over and 
over again to do its real computation work. 
Or it can execute its `post_run` function to finalize its life cycle, 
free all the private memory it allocated in its life cycle, unregister its
output tensors from the tensor table, and enter the **end** state, where the
LightNet context can safely remove this operator from the operator table and
the data flow graph.

`ln_op` has those 4 state-transfer functions in its C struct as function
pointers, which has the same prototype `ln_op_func`:

    :::c
    typedef void (*ln_op_func) (ln_op_arg *op_arg);
    typedef size_t (*ln_op_offset_func) (ln_op_arg *op_arg, ln_tensor_entry *te);
    
    struct ln_op {
        ln_op_arg          *op_arg;
        ln_op_func          pre_run;
        ln_op_func          static_run;
        ln_op_func          run;
        ln_op_func          post_run;
        ln_op_offset_func   calc_offset;
    };
    typedef struct ln_op ln_op;

Besides `ln_op_func`, there is another `ln_op_offset_func` used in some 
operators that hace to determine their output tensors' data address offsets
according to their input tensors.

Those functions all has a `ln_op_arg` as their argument, through which they can
access and manipulate the operators' input tensors (`tensors_in`), output tensors
(`tensors_out`) and parameters (`params`). Besides, `ln_op_arg` has a `priv` 
field reserved for the operators' private data. Operators can define
a private struct in their defination C file, stores its private data in it and
assign `priv` with the private struct's pointer to pass it on and use it in
different state-transfer functions.

    :::c
    struct ln_op_arg {
        char                 *name;           /* operator name */
        char                 *optype;         /* operator type */
        char                 *arch;           /* backend architecture to run on */
        ln_list              *tensors_in;     /* input tensors */
        ln_list              *tensors_out;    /* output tensors */
        ln_list              *params;         /* parameters */
        ln_hash              *tensor_table;   /* tensor table of the context */
        void                 *priv;           /* for other private data storage */
        const char          **in_arg_names;   /* NULL terminated array, as belows */
        const char          **out_arg_names;
        const char          **param_arg_names;
        const ln_param_type  *param_ptypes;
    };
    typedef struct ln_op_arg ln_op_arg;

`ln_op` generally supports the following operations:

- **`ln_op *ln_op_create_from_proto(const ln_op *op_proto, const char *name, ln_list *tensors_in, ln_list *tensors_out, ln_list *params, ln_hash *tensor_table)`**

    Create an operator from a "proto" operator `op_proto`. The newly created operator
    will have the same function pointers (`pre_run`, `run`, etc) as `op_proto`, with
    its own `name`, `tensors_in`, `tensors_out`, `params`, and a `tensor_table`
    pointer of the context passed from the caller. `op_proto` generally are found
    from the global hash table `LN_ARCH.op_proto_table` with its operator type as 
    the key.

- **`void ln_op_free(ln_op *op)`**

    Free an operator.

- **`void ln_op_free_lists_too(ln_op *op)`**

    Free an operator, as well as its `tensors_in`, `tensors_out` and `params`.

- **`ln_op *ln_op_create_with_names(const ln_op *op_proto, ln_hash *tensor_table)`**

    Create an operator and its `tensors_in`, `tensors_out` and `params` as well,
    with auto-generated unique operator name and output tensor names in the scope
    of the context. Input tensor names are inited with empty string (""), and
    parameters are inited with empty value (zeroed number or `NULL` string or
    `NULL` array). 
    The meta information of the operator used to create those 
    stuff is found in `ln_op_arg`'s `in_arg_names`, `out_arg_names`, 
    `param_arg_names`, `param_ptypes`, etc.
    This function is mainly used in optimization pass where LightNet will 
    generate optimized new operators to replace old ones.

- **`ln_op *ln_op_create_with_opname(const ln_op *op_proto, ln_hash *tensor_table)`**

    Create an operator with auto-generated unique operator name in the scope of
    the context.

- **`ln_op *ln_op_copy(const ln_op *op)`**

    Copy an operator.

- **`ln_op *ln_op_copy_to_optype(ln_hash *op_proto_table, const ln_op *op, const char *new_optype)`**

    Copy an operator's input/output tensors and parameters to a newly created
    operator of a new operator type `new_optype`. Generally used in the simple 
    replacement from one optype to another in optimization pass, such as
    replacing `conv` with `conv_cuda`.

- **`ln_tensor_entry *ln_op_find_tensor_entry(const ln_op *op, const char *arg_name)`**

    Find an operator's input or output tensor entry with the entry's `arg_name`.

- **`ln_tensor_list_entry *ln_op_find_tensor_list_entry(const ln_op *op, const char *arg_name)`**

    Find an operator's input or output tensor list entry with the entry's `arg_name`.

The are both an operator table and an operator list existing in a LightNet context.
While the former has the ownership of all operators, the latter retains a linear
form of the operators, which contains the execution order of the operators.

The operator list supports the following operations:

- **`ln_list *ln_op_list_create_from_array(ln_op **op_array)`**

    Create an operator list from an `NULL` terminated operator array.

- **`void ln_op_list_free(ln_list *op_list)`**

    Free an operator list.

- **`void ln_op_list_free_lists_too(ln_list *ops)`**

    Free an operator list, as well as the operators' tensor lists and
    parameter list.

- **`ln_op *ln_op_list_find_by_optype(ln_list *ops, const char *optype)`**

    Find the first operator from an operator list that has `optype` as its
    operator type.

- **`ln_op *ln_op_array_find_by_optype(ln_op *ops[], const char *optype)`**

    Find the first operator from an `NULL` terminated operator array that
    has `optype` as its operator type.

- **`ln_op *ln_op_list_find_by_name(ln_list *ops, const char *name)`**

    Find the first operator from an operator list that has `name` as its
    operator name.

- **`void ln_op_list_do_pre_run(ln_list *ops)`**

    Execute the `pre_run` functions of the operators in `ops` in order.

- **`void ln_op_list_do_static_run(ln_list *ops)`**

    Execute the `static_run` functions of the operators in `ops` in order.

- **`void ln_op_list_do_run(ln_list *ops)`**

    Execute the `run` functions of the operators in `ops` in order.

- **`void ln_op_list_do_post_run(ln_list *ops)`**

    Execute the `post_run` functions of the operators in `ops` in order.

- **`char *ln_op_list_new_opname(const ln_list *ops, const char *prefix)`**

    Create an unique operator name in the operator list `ops` with `prefix`.
    Return the newly allocated name string.

The operator table supports the following operations:

- **`ln_hash *ln_op_table_create(void)`**

    Create an empty operator table.

- **`int ln_op_table_insert(ln_hash *table, ln_op *op)`**

    Insert an operator to the table.

- **`int ln_op_table_remove(ln_hash *table, const char *name)`**

    Remove an operator from the table. The operator table will free the operator,
    its tensor lists and parameter list when removing it from the table.

- **`ln_op *ln_op_table_find(ln_hash *table, const char *name)`**

    Find an operator from the table with its name.

- **`void ln_op_table_free(ln_hash *table)`**

    Free an operator table, as well as its operators.

- **`void ln_op_table_vset_param(ln_hash *table, const char *opname, const char *pname, va_list ap)`**

    Set an operator's parameter in the operator table.
    Used in variable-length functions.

- **`void ln_op_table_set_param(ln_hash *table, const char *opname, const char *pname, ...)`**

    Set an operator's parameter with operator name `opname` in the operator table
    and parameter argument name `pname` in the parameter list.
    The function argument after `pname` should be a parameter value with an 
    appropriate data type consistent with the according parameter.

There are also a bunch of macros designated for the validity checking of tensors
and parameters in `pre_run` function. 

!!! warning

    Those convinent macros should **only** used in a `pre_run` function 
    because of the function arguments they required. 
    Normally we shouldn't use those kind of error handling routines in
    other state-transfer functions, where errors should be considered
    as bugs.
    If there is more error handling work, please write the code yourself
    instead of using those macros.

- **`ln_opck(level, condition, msg_fmt, varg...)`**

    If `condition` not satisfied, emit message defined by printf-liked arguments 
    `msg_fmt`and `varg...`, according to message level `level` (an enum defined in 
    `ln_msg.h`).

- **`ln_opck_satisfy_msg(condition, msg_fmt, varg...)`**

    Emit error message if `condition` not satisfied and exit.

- **`ln_opck_satisfy(condition)`**

    Emit error message if `condition` not satisfied and exit.

- **`ln_opck_param_exist(entry, arg_name)`**

    Check if a param entry exists. `entry` should be returned by
    `ln_param_list_find(op_arg->params, arg_name)`.

- **`ln_opck_param_type(entry, param_type)`**

    Check if a param entry's type is equal to `param_type`.
    `param_type` is an enum `of ln_param_type` defined in `ln_param.h`.
    `entry` should have been checked with `ln_opck_param_exist`.
    
The following macros check if a number param entry's value is equal to (`eq`), 
greater than (`gt`), greater than or equal to (`ge`), less than (`lt`),
less than or equal to (`le`), or not equal to (`ne`) the number `expect`.
`entry` should have been checked with `ln_opck_param_exist`.

- **`ln_opck_param_int_eq(entry, expect)`**
- **`ln_opck_param_int_gt(entry, expect)`**
- **`ln_opck_param_int_ge(entry, expect)`**
- **`ln_opck_param_int_lt(entry, expect)`**
- **`ln_opck_param_int_le(entry, expect)`**
- **`ln_opck_param_int_ne(entry, expect)`**
- **`ln_opck_param_float_eq(entry, expect)`**
- **`ln_opck_param_float_gt(entry, expect)`**
- **`ln_opck_param_float_ge(entry, expect)`**
- **`ln_opck_param_float_lt(entry, expect)`**
- **`ln_opck_param_float_le(entry, expect)`**
- **`ln_opck_param_float_ne(entry, expect)`**
- **`ln_opck_param_double_eq(entry, expect)`**
- **`ln_opck_param_double_gt(entry, expect)`**
- **`ln_opck_param_double_ge(entry, expect)`**
- **`ln_opck_param_double_lt(entry, expect)`**
- **`ln_opck_param_double_le(entry, expect)`**
- **`ln_opck_param_double_ne(entry, expect)`**

The following macros check if an array param entry's value's length is
equal to (`eq`), greater than (`gt`), greater than or equal to (`ge`), 
less than (`lt`), less than or equal to (`le`), or not equal to (`ne`)
the length `expect_len`. 
`entry` should have been checked with `ln_opck_param_exist`.

- **`ln_opck_param_array_len_eq(entry, expect_len)`**
- **`ln_opck_param_array_len_gt(entry, expect_len)`**
- **`ln_opck_param_array_len_ge(entry, expect_len)`**
- **`ln_opck_param_array_len_lt(entry, expect_len)`**
- **`ln_opck_param_array_len_le(entry, expect_len)`**

The following macros check if a number array param entry's values are all
equal to (`eq`), greater than (`gt`), greater than or equal to (`ge`), 
less than (`lt`), less than or equal to (`le`), or not equal to (`ne`) 
the number `expect`.
`entry` should have been checked with `ln_opck_param_exist`.

- **`ln_opck_param_array_int_eq(entry, expect)`**
- **`ln_opck_param_array_int_gt(entry, expect)`**
- **`ln_opck_param_array_int_ge(entry, expect)`**
- **`ln_opck_param_array_int_lt(entry, expect)`**
- **`ln_opck_param_array_int_le(entry, expect)`**
- **`ln_opck_param_array_int_ne(entry, expect)`**
- **`ln_opck_param_array_float_eq(entry, expect)`**
- **`ln_opck_param_array_float_gt(entry, expect)`**
- **`ln_opck_param_array_float_ge(entry, expect)`**
- **`ln_opck_param_array_float_lt(entry, expect)`**
- **`ln_opck_param_array_float_le(entry, expect)`**
- **`ln_opck_param_array_float_ne(entry, expect)`**
- **`ln_opck_param_array_double_eq(entry, expect)`**
- **`ln_opck_param_array_double_gt(entry, expect)`**
- **`ln_opck_param_array_double_ge(entry, expect)`**
- **`ln_opck_param_array_double_lt(entry, expect)`**
- **`ln_opck_param_array_double_le(entry, expect)`**
- **`ln_opck_param_array_double_ne(entry, expect)`**

The following macros check if the length of the parameter list (`list_len`)
is equal to (`eq`), greater than (`gt`), greater than or equal to (`ge`), 
less than (`lt`), less than or equal to (`le`), or not equal to (`ne`) 
the length `expect_len`.
`list_len` should be returned by `ln_param_list_length(op_arg->params)`.

- **`ln_opck_params_len_eq(list_len, expect_len)`**
- **`ln_opck_params_len_gt(list_len, expect_len)`**
- **`ln_opck_params_len_ge(list_len, expect_len)`**
- **`ln_opck_params_len_lt(list_len, expect_len)`**
- **`ln_opck_params_len_le(list_len, expect_len)`**

- **`ln_opck_tensor_in_exist(entry, arg_name)`**

    Check if an input tensor list entry exists. `entry` should be returned by
    `ln_tensor_list_find_by_arg_name(op_arg->tensors_in, arg_name)`.

The following macros check if the length of the input tensor list (`list_len`)
is equal to (`eq`), greater than (`gt`), greater than or equal to (`ge`), 
less than (`lt`), less than or equal to (`le`), or not equal to (`ne`) 
the length `expect_len`.
`list_len` should be returned by `ln_tensor_list_length(op_arg->tensors_in)`.

- **`ln_opck_tensors_in_len_eq(list_len, expect_len)`**
- **`ln_opck_tensors_in_len_gt(list_len, expect_len)`**
- **`ln_opck_tensors_in_len_ge(list_len, expect_len)`**
- **`ln_opck_tensors_in_len_lt(list_len, expect_len)`**
- **`ln_opck_tensors_in_len_le(list_len, expect_len)`**

- **`ln_opck_tensor_out_exist(entry, arg_name)`**

    Check if an output tensor list entry exists. `entry` should be returned by
    `ln_tensor_list_find_by_arg_name(op_arg->tensors_out, arg_name)`.
    
The following macros check if the length of the output tensor list (`list_len`)
is equal to (`eq`), greater than (`gt`), greater than or equal to (`ge`), 
less than (`lt`), less than or equal to (`le`), or not equal to (`ne`) 
the length `expect_len`.
`list_len` should be returned by `ln_tensor_list_length(op_arg->tensors_out)`.

- **`ln_opck_tensors_out_len_eq(list_len, expect_len)`**
- **`ln_opck_tensors_out_len_gt(list_len, expect_len)`**
- **`ln_opck_tensors_out_len_ge(list_len, expect_len)`**
- **`ln_opck_tensors_out_len_lt(list_len, expect_len)`**
- **`ln_opck_tensors_out_len_le(list_len, expect_len)`**

- **`ln_opck_tensor_not_defined(entry, entry_name)`**

    Check if the tensor entry `entry` has *not* been defined by another
    operator so far. "defined" means an operator has created the tensor
    and inserted the tensor entry in the tensor table.
    `entry` should be returned by
    `ln_tensor_table_find(op_arg->tensor_table, entry_name)`, where `entry_name`
    is the `ln_tensor_entry`'s name to be looked up.

- **`ln_opck_tensor_defined(entry, entry_name)`**

    Check if the tensor entry `entry` has been defined by another
    operator so far.
    `entry` should be returned by
    `ln_tensor_table_find(op_arg->tensor_table, entry_name)`, where `entry_name`
    is the `ln_tensor_entry`'s name to be looked up.

- **`ln_opck_tensor_ndim(entry, expect_ndim)`**

    Check if the tensor in `entry` has the number of dimensions equal to
    `expect_ndim`.
    `entry` should have been checked with `ln_opck_tensor_defined`.

- **`ln_opck_tensor_len(entry, expect_len)`**

    Check if the tensor in `entry` has the number of elements equal to
    `expect_len`.
    `entry` should have been checked with `ln_opck_tensor_defined`.

- **`ln_opck_tensor_issameshape(entry1, entry2)`**

    Check if the tensor in `entry1` has the same shape with the tensor in
    `entry2`.
    Both `entry1` and `entry2` should have been checked with `ln_opck_tensor_defined`.

- **`ln_opck_tensor_issametype(entry1, entry2)`**

    Check if the tensor in `entry1` has the same data type with the tensor in
    `entry2`.
    Both `entry1` and `entry2` should have been checked with `ln_opck_tensor_defined`.

- **`ln_opck_tensor_isstatic(entry)`**

    Check if the tensor in `entry` is static.
    `entry` should have been checked with `ln_opck_tensor_defined`.

- **`ln_opck_tensor_isnotstatic(entry)`**

    Check if the tensor in `entry` is *not* static.
    `entry` should have been checked with `ln_opck_tensor_defined`.

- **`ln_opck_tensor_mtype_eq(entry, mem_type)`**

    Check if the tensor in `entry` has the memory type equals to `mem_type`.
    `entry` should have been checked with `ln_opck_tensor_defined`.

- **`ln_opck_tensor_dtype_eq(entry, data_type)`**

    Check if the tensor in `entry` has the data type equals to `data_type`.
    `entry` should have been checked with `ln_opck_tensor_defined`.
    
## Data Flow Graph

LightNet uses Data Flow Graph (`ln_dfg`) to represent data flows among operators,
as known as computing graph.

    :::c
    struct ln_dfg {
        ln_graph *graph;
        ln_hash  *node_table;
        ln_list  *dangling_ins;
        ln_list  *dangling_outs;
    };
    typedef struct ln_dfg ln_dfg;

`ln_dfg` has a `graph` as its core data structure, with operators as nodes and
tensor names as edges. Besides, it has a `node_table` to manage all the graph nodes
in a hash table, keyed by operator names, a `dangling_outs` list to manage all 
the reaching out dangling edge nodes, and a `dangling_ins` list to manage all 
the reaching in dangling edge nodes.

`ln_dfg` supports the following operations:

- **`ln_dfg *ln_dfg_create(void)`**

    Create an empty data flow graph.

- **`void ln_dfg_free(ln_dfg *dfg)`**

    Free a data flow graph, as well as its `graph`, `node_table`, `dangling_ins`, 
    `dangling_outs`.
    
!!!note
    The operator pointers held by graph nodes are owned by `op_table` of the 
    context. So freeing `graph` and `node_table` doesn't actually free the
    underlying operators. (TODO: May be the graph nodes holding operator names is 
    well enough?)

- **`void ln_dfg_link(ln_dfg *dfg, ln_op *op1, ln_op *op2, const char *tname)`**

    Link the node holding `op1` to the node holding `op2`, with tensor name `tname`
    as the edge.

- **`void ln_dfg_unlink(ln_dfg *dfg, ln_op *op1, ln_op *op2, const char *tname)`**

    Unlink the node holding `op1` to the node holding `op2`, with tensor name `tname`
    as the edge. If `tname` is not the output tensor of `op1` or not the input
    tensor of `op2`, this function does nothing.

- **`void ln_dfg_add(ln_dfg *dfg, ln_op *op)`**

    Add `op` to the `dfg`. The new node automatically gets linked with its output
    tensors to nodes that take them as input tensors (if any), and gets linked 
    with its input tensors from nodes that take them as output tensors (if any),
    and removes/adds related entries in the `dfg`'s `dangling_outs` and
    `dangling_ins`.
    
- **`void ln_dfg_remove(ln_dfg *dfg, ln_op *op)`**

    Remove `op` to the `dfg`. The node automatically gets unlinked with its output
    tensors to nodes that take them as input tensors (if any), and gets unlinked 
    with its input tensors from nodes that take them as output tensors (if any),
    and removes/adds related entries in the `dfg`'s `dangling_outs` and
    `dangling_ins`.

- **`ln_op *ln_dfg_next(const ln_dfg *dfg, const ln_op *op, const char *tname)`**

    (Deprecated)
    Return the operator that has `tname` as its input tensor's name, which is also
    operator `op`'s output tensor.

- **`ln_list *ln_dfg_nexts(const ln_dfg *dfg, const ln_op *op, const char *tname)`**
  
    Return all operators that have `tname` as their input tensor's name, which is also
    operator `op`'s output tensor.

- **`ln_op *ln_dfg_prev(const ln_dfg *dfg, const ln_op *op, const char *tname)`**

    Return the operator that has `tname` as its output tensor's name, which is also
    operator `op`'s input tensor.

- **`int ln_dfg_check(const ln_dfg *dfg)`**

    Check the correctness of a data flow graph `dfg`. It emits an internal error if
    any operator's input tensor is not given by another operator.

- **`void ln_dfg_fprint(FILE *fp, const ln_dfg *dfg)`**

    Print the `dfg` to file stream `fp`.

- **`void ln_dfg_print(const ln_dfg *dfg)`**

    Print the `dfg` to stdout.

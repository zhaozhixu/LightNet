# Intermediate Representation

## JSON IR Format

LightNet uses a JSON formatted intermediate representation (IR) to represent
NN models, which is conforming to the following format:

    :::js
    {
        "ops": [
            {
                "name": STRING,
                "optype": STRING,
                "tensors_in": [
                    {
                        "arg_name": STRING,
                        "name": STRING
                    },
                    ...
                ],
                "tensors_out": [
                    {
                        "arg_name": STRING, 
                        "name": STRING
                    },
                    ...
                ],
                "params": [
                    {
                        "arg_name": STRING,
                        "value": STRING or NUMBER or BOOL or ARRAY of <STRING or NUMBER or BOOL>
                    },
                    ...
                ]
            },
            ...
        }
    }

The IR format is very simple. It contains an array of operators `ops`, in which
every operator is a JSON object. An operator should has the following fields:

- `name`: A string name that is unique among all operators.
- `optype`: A string of operator type that denotes the operation it performs.
- `tensors_in`: The input tensors of the operator.
- `tensors_out`: The output tensors of the operator.
- `params`: The parameters of the operator.

 See [Operator](Data-Structures.md#operator) for the corresponding C data
 structures of the above fields used by LightNet .

`tensors_in`, `tensors_out` and `params` all have a `arg_name` field. If you think the
operator as a C function, then `arg_name` is the argument name of that function.
Thus, `arg_name` must be unique in the scope of the operator.

The `name` field in a tensor is the tensor's unique name among all tensors,
which is used to look up the tensor in the tensor table. New tensors can only be
defined in output tensors, and input tensors must have been defined by previous
operators in their output tensors.
See [Tensor](Data-Structures.md#tensor) for details of the underlying data 
structures.

The `value` field in a paramenter is its value, of the data type that the 
paramenter requires. See [Parameter](Data-Structures.md#parameter) for
the possible data types and the underlying data structures used by parameters.

## Example

The following code is an example of a simple IR composed of 3 operators: `create1`,
`slice1`, and `print1`. `create1` first creates a tensor `tensor1` of shape
2 x 4, with float data [1, 2, 3, 4, 5, 6, 7, 8], then `slice1` slices the tensor
in its second axis (axis starts from 0), resulting to a tensor `tensor2` of
shape 2 x 3. Finally `print1` prints `tensor2` to stdout.

    :::js
    {
        "ops": [
            {
                "name": "create1",
                "optype": "create",
                "tensors_in": [
                ],
                "tensors_out": [
                    {"arg_name": "dst", "name": "tensor1"}
                ],
                "params": [
                    {"arg_name": "dtype", "value": "TL_FLOAT"},
                    {"arg_name": "dims", "value": [2, 4]},
                    {"arg_name": "data", "value": [1, 2, 3, 4, 5, 6, 7, 8]},
                    {"arg_name": "ran", "value": [0, 0]},
                    {"arg_name": "from_file", "value": false}
                ]
            },
            {
                "name": "slice1",
                "optype": "slice",
                "tensors_in": [
                    {"arg_name": "src", "name": "tensor1"}
                ],
                "tensors_out": [
                    {"arg_name": "dst", "name": "tensor2"}
                ],
                "params": [
                    {"arg_name": "axis", "value": 1},
                    {"arg_name": "start", "value": 1},
                    {"arg_name": "len", "value": 3}
                ]
            },
            {
                "name": "print1",
                "optype": "print",
                "tensors_in": [
                    {"arg_name": "src", "name": "tensor2"}
                ],
                "tensors_out": [
                ],
                "params": [
                    {"arg_name": "msg", "value": "tensor2:"}
                ]
            }
        ]
    }

If the above code is saved in `example.json`, executing the following 
command will gives `print1`'s message:

    $ lightnet example.json
    tensor2:
    [[2.000 3.000 4.000]
     [6.000 7.000 8.000]]
    info: run time: 0.000019s

# Intermediate Language

## IL Format

The [Intermediate Representation](Intermediate-Representation.md) may seems
simple at the first glance. But when we're writing a real neural network,
the model file in JSON IR format is easily getting tens of thousands of lines,
which is still piece of cake for computers, but a nightmare for humans.

Thus, LightNet provides a "concise version" of JSON IR format, which contains
the exactly the same information as JSON IR, called LightNet Intermediate
Language (IL), which can be transformed to JSON IR by `il2json.pl` (installed
by default).

The basic format for an operator's representation in IL is:

    :::c
    OPTYPE(TENSOR_IN_ARG_NAME1=TENSOR_IN_NAME1, 
           TENSOR_IN_ARG_NAME2=TENSOR_IN_NAME2, ... |
           TENSOR_OUT_ARG_NAME1=TENSOR_OUT_NAME1,
           TENSOR_OUT_ARG_NAME1=TENSOR_OUT_NAME1, ... |
           PARAM_ARG_NAME1=PARAM_VALUE1,
           PARAM_ARG_NAME2=PARAM_VALUE2, ...);
           
For example, [the example IR used in Intermediate Representation section](Intermediate-Representation.md#example)
that composed of 3 operators (`create1`, `slice1`, `print1`) can also be written
in IL:

    :::c
    create(| dst=tensor1 | dtype=TL_FLOAT, dims=[2, 4],
           data=[1, 2, 3, 4, 5, 6, 7, 8], ran=[0, 0], from_file=false);
    slice(src=tensor1 | dst=tensor2 | axis=1, start=1, len=3);
    print(src=tensor2 | | msg="tensor2:");
    
Save the above code in `example.net`, and `il2json.pl` can generate the same code
as in the [example IR](Intermediate-Representation.md#example):

    $ il2json.pl example.net -o example.json

Or you can combine `ir2json` and `lightnet` with the pipe:
    
    $ il2json.pl example.net | lightnet -
    tensor2:
    [[2.000 3.000 4.000]
    [6.000 7.000 8.000]]
    info: run time: 0.000062s

The IL interpreter utlizes the C preprocessor of GCC, thus all C preprocessor
syntaxes are legal in IL.

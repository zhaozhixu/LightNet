# Intermediate Language

## Basic IL Format

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

Then you can feed `example.json` directly to `lightnet`. Or you can just combine 
`il2json` and `lightnet` with a pipe:
    
    $ il2json.pl example.net | lightnet -
    tensor2:
    [[2.000 3.000 4.000]
     [6.000 7.000 8.000]]
    info: run time: 0.000062s

## Wrap Operators with Macros

The IL interpreter utlizes the C preprocessor of GCC, thus all C preprocessor
syntaxes are legal in an IL.

For example, it is common to have a `conv2d` and a `relu` operator in a row in
NN models. Instead of writing `conv2d` and `relu` repeatedly, we can use
`#define` directive to wrap them in a macro for future uses.

    :::c
    #define conv(in_name, out_name, in_c, out_c,                            \
                 _size, _stride, _padding, _dilation)                       \
        create(| dst=out_name##_wts |                                       \
               dtype=TL_FLOAT, dims=[out_c, in_c, _size, _size],            \
               ran=[-10, 10], data=[0], from_file=true);                    \
        create(| dst=out_name##_bias | dtype=TL_FLOAT, dims=[out_c],        \
               ran=[-10, 10], data=[0], from_file=true);                    \
        conv2d(src=in_name, weight=out_name##_wts, bias=out_name##_bias |   \
               dst=out_name | group=1, size=[_size, _size],                 \
               stride=[_stride, _stride],                                   \
               padding=[_padding, _padding, _padding, _padding],            \
               autopad="NOTSET",                                            \
               dilation=[_dilation, _dilation])

    #define conv_relu(in_name, out_name, in_c, out_c,       \
                      _size, _stride, _padding, _dilation)  \
        conv(in_name, out_name##_conv, in_c, out_c,         \
             _size, _stride, _padding, _dilation);          \
        relu(src=out_name##_conv | dst=out_name |)

The above code first defined a `conv` macro, which consists of two `create`
operators and a `conv2d` operator. The two `create`s create the tensors
for the weight and the bias of the `conv2d`. The `conv2d` operator does the
convolution algorithm using the input tensor named `in_name`, to the output
tensor named `out_name`, with a weight tensor named `out_name##wts` and a
bias tensor named `out_name##bias`. The `##` is the string-concatation operator
in the C preprocessor which concat the macro parameter `out_name` with `wts` 
or `bias`, so that when we calls `conv(tensor1, tensor2,...)`, the weight 
tensor gets `tensor2_wts` and the bias tensor gets `tensor2_bias` as their names
automatically.

Some readers may notice that some macro parameters are prefixed with a `_`.
That's to prevent the parameter names from conflicting with the argument
names of the operators in the macro, such as the `padding` argument of `conv2d`.

The code then defined a `conv_relu` macro, which calls the
former `conv` macro and feeds its output to the input of a `relu` operator.

## Embeded Scripts

Sometimes we want some constant expressions to be calculated before we actually
translate the IL, which cannot be done by the C preprocessor.
So after the C preprocessor, IL supports embeded scripts to process the IL text
further.

Embeded scripts are enclosed in `${eval ... }` expressions. Suppose we needs
to get the anchor number in an object detection network 
(such as [SqueezeDet](https://arxiv.org/abs/1612.01051)), whose anchor number can be
calculated as

    anchor_num = anchors_per_grid * grid_height * grid_width
    
Using embeded scripts, this may be written as:

    :::c
    #define ANCHORS_PER_GRID 9
    #define CONVOUT_H 12
    #define CONVOUT_W 20
    #define ANCHOR_NUM ${eval ANCHOR_PER_GRID * CONVOUT_H * CONVOUT_W}

Embeded scripts are actually Perl code snippets. So in principle, any legal Perl
code can be run in an `eval` expression.

!!!warning
    Since embeded scripts are Perl code, be cautious that malicious code may
    do nasty things to your system if an untrusty IL is interpreted with
    privileged priorities. Of course, there is **never** a good reason for an IL
    text to be translated with privileges.

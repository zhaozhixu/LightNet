# Miscellaneous

## An Easy-to-Write JSON Format

For the convenience of writing JSON format files such as 
[operator descriptions](Operator-Description.md) used in this project, 
we add several extensions to the [official JSON format](http://json.org/).

The JSON extensions:

1. The last elements in arrays and objects can have a trailing comma `,`.
2. Comments are supported; every thing from double slashs `//` to the end of
   the line is a comment.
3. String values can be heredoc-ed with two triple qoutes `'''`, each in a
   seperated line, between which special characters can be auto-escaped,
   such as newlines, qoutes`"`, etc. The triple qoutes must be at the end of a
   line except that the ending qoutes can have a comma following them.
   
E.g:

    :::js
    {
        "some_array": [
            {"element1": 1},
            {"element2": 2},
        ],
        // some very useful comments
        "some_string": '''
        No worries about "strings" and
    newlines here!
    ''',
    }
   
JSON files using above extensions can be easily transformed into official JSON 
files with `lightnet/tools/easyjson.pl`. For example, the above code
would be transformed into the following JSON:

    :::js
    {
        "some_array": [
            {"element1": 1},
            {"element2": 2}
        ],
    
        "some_string": "    No worries about \"strings\" and\nnewlines here!"
    }

## Data File Format

## Tools

Executables, scripts and modules helping the development and use of LightNet, 
located in `lightnet/tools`.
You can always get help for a tool by typing `tool_name -h`.

* `addop.pl`

    Generate operator defination code from operator description JSON file.
    It supports the [easy-to-write JSON format](Miscellaneous.md#an-easy-to-write-json-format)
    for its input.
    When used with `-r ROOT` option, it will save operator definations in
    `ROOT/src/op/auto`, and add operator declarations into corresponding
    `ROOT/src/arch/ln_arch_*.c` file.

* `addtest.pl`

    Generate test templates for a module.

* `easyjson.pl`

    A user interface for `easyjson.pm`.

* `easyjson.pm`

    A perl module with interfaces for transforming an 
    [easy-to-write JSON format](Miscellaneous.md#an-easy-to-write-json-format)
    file to an official JSON format file.

* `genwts.pl`

    A weight file generator. Convert the input text file containing weight
    numbers to one text file in hexadecimal string format.

* `il2json`

    Generate JSON-format IR code from input file which is in 
    [simplified IR format]().


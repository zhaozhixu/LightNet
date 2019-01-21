# Miscellaneous

## An Easy-to-Write JSON Format

For the convenience of write JSON format files (operator descriptions, IR etc.)
used in this project, we add several extensions to the [official JSON format](http://json.org/).

The JSON extensions:

1. The last elements in arrays and objects can have a trailing comma `,`.
2. Comments are supported; every thing from double slashs `//` to the end of
   the line is a comment.
3. String values can be heredoc-ed with two triple qoutes `'''`, each in a
   seperated line, between which special characters can be auto-escaped,
   such as newlines, qoutes`"`, etc. The triple qoutes must be at the end of a
   line except that the ending qoutes can have a comma following them.
   
JSON files using above extensions can be easily transformed into official JSON 
files with [easyjson.pl](Tools#easyjson.pl)


## Tools

Executables, scripts and modules helping the development and use of LightNet, 
located in `lightnet/tools`.
You can always get help for a tool by typing `tool_name -h`.

* `addop.pl`

    Generate operator defination code from operator description JSON file.
    When used with `-r ROOT` option, it will save operator definations in
    `ROOT/src/op/auto`, and add operator declarations into corresponding
    `ROOT/src/arch/ln_arch_*.c` file.

* `addop_simple.pl`

    Not used now. An old simple script for operator code generation.

* `addtest.pl`

    Generate test templates for a module.

* `easyjson.pl`

    A user interface for `easyjson.pm`.

* `easyjson.pm`

    A perl module with interfaces for transforming an 
    [easy-to-write JSON format](Miscellaneous.md#An-Easy-to-Write-JSON-Format)
    file to an official JSON form file.

* `genwts.pl`

    A weight file generator. Convert the input text file containing weight
    numbers to one text file in hexadecimal string format.

* `ir2json.pl`

    Generate JSON-format IR code from input file which is in 
    [simplified IR format]().


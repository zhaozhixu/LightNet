#! /usr/bin/perl

use 5.014;
use warnings;
use strict;
use JSON;
use File::Copy;
use Cwd 'abs_path';

my $usage = <<EOF;
Usage: $0 ROOT OP_JSON_FILE;
ROOT is the path of the project root.
OP_JSON_FILE is the location of op defination file.

Example:
	tools/addop_extend.pl . slice.json

	Executing this from project root will generate code templates
	in file ROOT/src/ln_opimpl_slice_cuda.c from slice.json, and add
	associated init ops in ROOT/src/ln_arch_cpu.c.
EOF
if (@ARGV < 2) {
    print $usage;
    exit;
}
my $root = abs_path($ARGV[0]);
my $json_file = $ARGV[1];
open JSON_FILE, '<', $json_file or die "Cannot open $json_file: $!";
my $json_text = join '', <JSON_FILE>;
close JSON_FILE;

my $json = decode_json $json_text;
&gen_op($json->{ops}[0]);

sub gen_op {
    my $op = $_[0];
    my $optype = $op->{optype};
    my $subfix = "";
    $subfix = $op->{subfix} if exists $op->{subfix};
    my $tensors_in = $op->{tensors_in};
    my $tensors_out = $op->{tensors_out};
    my $params = $op->{params};

    if ($subfix ne "" and not $optype =~ /\w+_$subfix/) {
        &err_exit("optype \"$optype\" doesn't match subfix \"$subfix\"");
    }
    if ($subfix ne "" && $subfix ne "cuda") {
        &err_exit("unsupported subfix \"$subfix\"");
    }
    if ($subfix eq "") {
        $subfix = "cpu";
    }

    my $struct_def = &gen_struct_def($op);
    my $pre_run_local_vars = &gen_pre_run_local_vars($op);
    my $pre_run_checks = &gen_pre_run_checks($op);
    my $output_tensor_def = &gen_output_tensor_def($op);
    my $priv_assigns = "";
    my $post_run_code = "";

    my $op_tpl = <<EOF;
/*
 * Copyright (c) 2018 Zhao Zhixu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <assert.h>
#include "ln_op.h"

${struct_def}
/*
 * This function should do the parameter checking and tensor shape inference.
 */
static void ${optype}_pre_run(ln_op_arg *op_arg, ln_error **error)
{
${pre_run_local_vars}

     /* check tensors and parameters */
${pre_run_checks}

     /* define output tensor shape, tensor data should be NULL */
${output_tensor_def}

     /* use op_arg->priv to store private data to be used in other functions */
${priv_assigns}
}

/*
 * This function should only do the calculations.
 */
static void ${optype}_run(ln_op_arg *op_arg, ln_error **error)
{
     /* TODO: add ${optype}_run */
}

/*
 * This function should free all the memory allocated by other *_run()s.
 */
static void ${optype}_post_run(ln_op_arg *op_arg, ln_error **error)
{
${post_run_code}
}

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_${optype} = {
     .optype = "${optype}",
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_${optype} = {
     .op_arg = &op_arg_${optype},
     .pre_run = ${optype}_pre_run,
     .static_run = NULL,
     .run = ${optype}_run,
     .post_run = ${optype}_post_run
};
EOF
    print $op_tpl;
}

sub gen_struct_def {
    my $op = $_[0];
    my $tensors_in = $op->{tensors_in};
    my $tensors_out = $op->{tensors_out};
    my $params = $op->{params};

    my @defs = ();
    foreach (@$tensors_in) {
        push @defs, "tl_tensor *$_->{arg_name};";
    }
    foreach (@$tensors_out) {
        push @defs, "tl_tensor *$_->{arg_name};";
        push @defs, "char *$_->{arg_name}_name;";
    }
    &gen_params($params, \@defs);
    &make_defs_neat(5, \@defs);

    my $defs_str = join "\n", @defs;
    my $struct_def_tpl = <<EOF;
struct priv_s {
${defs_str}
};
EOF
}

sub gen_pre_run_local_vars {
    my $op = $_[0];
    my $tensors_in = $op->{tensors_in};
    my $tensors_out = $op->{tensors_out};
    my $params = $op->{params};

    my @vars = ();
    foreach (@$tensors_in) {
        push @vars, "char *$_->{arg_name}_name;";
        push @vars, "ln_tensor_entry *$_->{arg_name}_entry;";
        push @vars, "tl_tensor *$_->{arg_name};";
    }
    foreach (@$tensors_out) {
        push @vars, "char *$_->{arg_name}_name;";
        push @vars, "ln_tensor_entry *$_->{arg_name}_entry;";
        push @vars, "tl_tensor *$_->{arg_name};";
        push @vars, "int $_->{arg_name}_ndim;";
        push @vars, "int *$_->{arg_name}_dims;";
        push @vars, "tl_dtype $_->{arg_name}_dtype;";
    }
    &gen_params($params, \@vars);

    push @vars, "int tensors_in_n;";
    push @vars, "int tensors_out_n;";
    push @vars, "int params_n;";
    push @vars, "struct priv_s *priv;";
    &make_defs_neat(5, \@vars);

    my $vars_str = join "\n", @vars;
}

sub gen_params {
    my $params = shift;
    my $defs = shift;
    foreach my $param (@$params) {
        my $realtype;
        given ($param->{ptype}) {
            when ("LN_PARAM_NULL") {
                $realtype = "void *";
            }
            when ("LN_PARAM_STRING") {
                if (exists $param->{realtype}) {
                    $realtype = $param->{realtype};
                } else {
                    $realtype = "char *";
                }
            }
            when ("LN_PARAM_NUMBER") {
                &err_exit("need a `realtype`") unless exists $param->{realtype};
                if ($param->{realtype} eq "float" ||
                    $param->{realtype} eq "double"||
                    $param->{realtype} eq "int") {
                    $realtype = $param->{realtype};
                } else {
                    &err_exit("unsupported `realtype`: '$param->{realtype}'");
                }
            }
            when ("LN_PARAM_BOOL") {
                $realtype = "tl_bool_t";
            }
            when ("LN_PARAM_ARRAY_STRING") {
                if (exists $param->{realtype}) {
                    $realtype = "$param->{realtype} *";
                } else {
                    $realtype = "char **";
                }
            }
            when ("LN_PARAM_ARRAY_NUMBER") {
                &err_exit("need a `realtype`") unless exists $param->{realtype};
                if ($param->{realtype} eq "float" ||
                    $param->{realtype} eq "double"||
                    $param->{realtype} eq "int") {
                    $realtype = "$param->{realtype} *";
                } else {
                    &err_exit("unsupported `realtype`: '$param->{realtype}'");
                }
            }
            when ("LN_PARAM_ARRAY_BOOL") {
                $realtype = "tl_bool_t *";
            }
            default {
                &err_exit("unsupported `ptype`: '$param->{ptype}'");
            }
        }
        $realtype .= " " unless ($realtype =~ /\*$/);

        push @$defs, "${realtype}$param->{arg_name};";
    }
}

sub gen_pre_run_checks {
    my $op = $_[0];
    my $tensors_in = $op->{tensors_in};
    my $tensors_out = $op->{tensors_out};
    my $params = $op->{params};

    my @states = ();
    my $tensors_in_n = @$tensors_in;
    push @states, "tensors_in_n = ln_tensor_list_length(op_arg->tensors_in);";
    push @states, "ln_opck_tensor_in_len_eq(tensors_in_n, ${tensors_in_n});";
    push @states, "";
    foreach my $tensor (@$tensors_in) {
        my $arg_name = $tensor->{arg_name};
        push @states, "${arg_name}_name = ln_tensor_list_find_name(op_arg->tensors_in, \"${arg_name}\");";
        push @states, "ln_opck_tensor_in_exist(${arg_name}_name, \"${arg_name}\");";
        push @states, "${arg_name}_entry = ln_tensor_table_find(op_arg->tensor_table, ${arg_name}_name);";
        push @states, "ln_opck_tensor_defined(${arg_name}_entry, ${arg_name}_name);";
        push @states, "ln_opck_tensor_mtype_eq(${arg_name}_entry, $tensor->{mtype});";
        if (exists $tensor->{dtype}) {
            push @states, "ln_opck_tensor_dtype_eq(${arg_name}_entry, $tensor->{dtype});";
        }
        if (exists $tensor->{sametype}) {
            push @states, "ln_opck_tensor_issametype(${arg_name}_entry, $tensor->{sametype}_entry);";
        }
        if (exists $tensor->{sameshape}) {
            push @states, "ln_opck_tensor_issameshape(${arg_name}_entry, $tensor->{sametype}_entry);";
        }
        if (exists $tensor->{static}) {
            if ($tensor->{static}) {
                push @states, "ln_opck_tensor_isstatic(${arg_name}_entry);";
            } else {
                push @states, "ln_opck_tensor_isnotstatic(${arg_name}_entry);";
            }
        }
        if (exists $tensor->{check}) {
            push @states, "ln_opck_tensor_satisfy_msg($tensor->{check});";
        }
        if (exists $tensor->{checks}) {
            my $checks = $tensor->{checks};
            foreach (@$checks) {
                if (exists $_->{check}) {
                    push @states, "ln_opck_tensor_satisfy_msg($_->{check});";
                } else {
                    &err_exit("tensor '${arg_name}' expects a `check` in `checks`");
                }
            }
        }
        if (exists $tensor->{custom}) {
            &add_custom_block($tensor->{custom}, \@states);
        }
        push @states, "";
    }

    my $tensors_out_n = @$tensors_out;
    push @states, "tensors_out_n = ln_tensor_list_length(op_arg->tensors_out);";
    push @states, "ln_opck_tensor_out_len_eq(tensors_out_n, ${tensors_out_n});";
    push @states, "";
    foreach my $tensor (@$tensors_out) {
        my $arg_name = $tensor->{arg_name};
        push @states, "${arg_name}_name = ln_tensor_list_find_name(op_arg->tensors_out, \"${arg_name}\");";
        push @states, "ln_opck_tensor_in_exist(${arg_name}_name, \"${arg_name}\");";
        push @states, "${arg_name}_entry = ln_tensor_table_find(op_arg->tensor_table, ${arg_name}_name);";
        push @states, "ln_opck_tensor_not_defined(${arg_name}_entry, ${arg_name}_name);";
        push @states, "";
    }

    my $params_n = @$params;
    push @states, "params_n = ln_param_list_length(op_arg->params);";
    push @states, "ln_opck_param_len_eq(params_n, ${params_n});";
    push @states, "";
    foreach my $param (@$params) {
        my $arg_name = $param->{arg_name};
        push @states, "${arg_name}_entry = ln_param_list_find(op_arg->params, \"${arg_name}\");";
        push @states, "ln_opck_param_exist(${arg_name}_entry, \"${arg_name}\");";
        push @states, "ln_opck_param_type(${arg_name}_entry, $param->{ptype});";
        given ($param->{ptype}) {
            when ("LN_PARAM_NULL") {
                push @states, "${arg_name} = NULL;";
            }
            when ("LN_PARAM_STRING") {
                if (exists $param->{realtype}) {
                    if (exists $param->{from_func}) {
                        push @states, "${arg_name} = $param->{from_func}(${arg_name}_entry->value_string);";
                    } else {
                        &err_exit("needs a `from_func` to convert '${arg_name}'");
                    }
                } else {
                    push @states, "${arg_name} = ${arg_name}_entry->value_string;";
                }
            }
            when ("LN_PARAM_NUMBER") {
                if ($param->{realtype} eq "float" ||
                    $param->{realtype} eq "double"||
                    $param->{realtype} eq "int") {
                    push @states, "${arg_name} = ${arg_name}_entry->value_$param->{realtype};";
                } else {
                    &err_exit("unsupported `realtype`: '$param->{realtype}'");
                }
            }
            when ("LN_PARAM_BOOL") {
                push @states, "${arg_name} = ${arg_name}_entry->value_bool;";
            }
            when (/^LN_PARAM_ARRAY/) {
                if (exists $param->{len}) {
                    push @states, "ln_opck_param_array_len_eq(${arg_name}_entry, $param->{len});";
                }
                continue ;
            }
            when ("LN_PARAM_ARRAY_STRING") {
              if (exists $param->{realtype}) {
                if (exists $param->{from_func}) {
                  push @states, "${arg_name} = ln_alloc(sizeof($param->{realtype})*${arg_name}_entry->array_len);";
                  push @states, "for (int i = 0; i < ${arg_name}_entry->array_len; i++)";
                  push @states, &indent_line(5, "${arg_name}[i] = $param->{from_func}(${arg_name}_entry->value_array_string[i]);");
                } else {
                  &err_exit("needs a `from_func` to convert '${arg_name}'");
                }
              } else {
                push @states, "${arg_name} = ${arg_name}_entry->value_array_string;";
              }
            }
            when ("LN_PARAM_ARRAY_NUMBER") {
              if ($param->{realtype} eq "float" ||
                  $param->{realtype} eq "double"||
                  $param->{realtype} eq "int") {
                push @states, "${arg_name} = ${arg_name}_entry->value_array_$param->{realtype};";
              } else {
                &err_exit("unsupported `realtype`: '$param->{realtype}'");
              }
            }
            when ("LN_PARAM_ARRAY_BOOL") {
              push @states, "${arg_name} = ${arg_name}_entry->value_array_bool;";
            }
            default {
                &err_exit("unsupported `ptype`: '$param->{ptype}'");
            }
        }
        if (exists $param->{check}) {
            push @states, "ln_opck_param_satisfy_msg($param->{check});";
        }
        if (exists $param->{checks}) {
            my $checks = $param->{checks};
            foreach (@$checks) {
                if (exists $_->{check}) {
                    push @states, "ln_opck_param_satisfy_msg($_->{check});";
                } else {
                    &err_exit("param '${arg_name}' expects a `check` in `checks`");
                }
            }
        }
        if (exists $param->{custom}) {
            &add_custom_block($param->{custom}, \@states);
        }
        push @states, "";
    }

    if (exists $op->{extra_checks}) {
        my $checks = $op->{extra_checks};
        foreach (@$checks) {
            if (exists $_->{cktype} && exists $_->{check}) {
                if ($_->{cktype} ne "param" && $_->{cktype} ne "tensor") {
                    &err_exit("`cktype` should be 'param' or 'tensor' in `extra_checks`");
                }
                push @states, "ln_opck_$_->{cktype}_satisfy_msg($_->{check});";
            } else {
                &err_exit("expects a `cktype` and a `check` in `extra_checks`");
            }
        }
        push @states, "";
    }
    if (exists $op->{extra_custom}) {
        &add_custom_block($op->{extra_custom}, \@states);
    }

    &indent_block(5, \@states);
    my $checks_str = join "\n", @states;
}

sub gen_output_tensor_def {
    my $op = $_[0];
    my $tensors_out = $op->{tensors_out};

    my @states = ();
    foreach my $tensor (@$tensors_out) {
        my $arg_name = $tensor->{arg_name};
        if (exists $tensor->{ndim}) {
            push @states, "${arg_name}_ndim = $tensor->{ndim};";
        } elsif (not exists $tensor->{custom}) {
            &err_exit("'${arg_name}' needs a `ndim` or `custom` to give the defination of ${arg_name}_ndim");
        }
        if (exists $tensor->{dims}) {
            push @states, "${arg_name}_dims = $tensor->{dims};";
        } elsif (not exists $tensor->{custom}) {
            &err_exit("'${arg_name}' needs a `dims` or `custom` to give the defination of ${arg_name}_dims");
        }
        if (exists $tensor->{dtype}) {
            push @states, "${arg_name}_dtype = $tensor->{dtype};";
        } elsif (not exists $tensor->{custom}) {
            &err_exit("'${arg_name}' needs a `dtype` or `custom` to give the defination of ${arg_name}_dtype");
        }
        if (exists $tensor->{custom}) {
            &add_custom_block($tensor->{custom}, \@states);
        }
        push @states, "${arg_name} = tl_tensor_create(NULL, ${arg_name}_ndim, ${arg_name}_dims, ${arg_name}_dtype);";
        push @states, "${arg_name}_entry = ln_tensor_entry_create(${arg_name}_name, ${arg_name});";
        if (exists $tensor->{mtype}) {
            push @states, "${arg_name}_entry->mtype = $tensor->{mtype};";
        } else {
            &err_exit("${arg_name} needs a `mtype`");
        }
        push @states, "ln_tensor_table_insert(op_arg->tensor_table, ${arg_name}_name, ${arg_name}_entry);";
        if (exists $tensor->{cleanup}) {
            &add_custom_block($tensor->{cleanup}, \@states);
        }
        push @states, "";
    }

    &indent_block(5, \@states);
    my $checks_str = join "\n", @states;
}

sub add_custom_block {
    my $custom_str = shift;
    my $states = shift;
    my @customs = split "\n", $custom_str;
    &indent_block(5, \@customs);
    push @$states, "{";
    push @$states, @customs;
    push @$states, "}";
}

sub make_defs_neat {
    my $nspaces = shift;
    my $defs = shift;
    my $max_offset = 0;
    foreach (@$defs) {
        if (/( |\*)(\w+;)/) {
            my $offset = index($_, $2);
            $max_offset = $max_offset < $offset ? $offset : $max_offset;
        } else {
            &err_exit("make_defs_neat: wrong format: $_");
        }
    }
    foreach (@$defs) {
        my $type = $1 if /((\w+[ \t]+)+)/;
        $type =~ s/[ \t]+$//;
        my $nstars = 0;
        $nstars = length $1 if /(\*+)/;
        my $rest = $2 if /( |\*)(\w+;)/;
        $_ = sprintf("%-${max_offset}s", $type);
        my $re = " "x$nstars;
        my $stars = "*"x$nstars;
        s/$re$/$stars$rest/;
    }
    &indent_block($nspaces, $defs);
}

sub indent_block {
    my $nspaces = shift;
    my $states = shift;
    $_ = " "x$nspaces.$_ foreach @$states;
}

sub indent_line {
    my $nspaces = shift;
    my $state = shift;
    $state = " "x$nspaces.$state;
}

sub err_exit {
    my $msg = $_[0];
    print STDERR "ERROR: $msg\n";
    exit 1;
}

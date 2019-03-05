#! /usr/bin/env perl

package testgl;

use 5.014;
use warnings;
use strict;
use JSON;
use File::Copy;
use Cwd 'abs_path';
use Getopt::Long;
use Scalar::Util qw(reftype);
use File::Basename;
use lib abs_path(dirname(__FILE__));
use util;
use easyjson;
use constant HASH => ref {};
use constant ARRAY => ref [];
use constant CODE => ref sub{};
no warnings 'experimental::smartmatch';

my %global_ops;
# my %defined_ops = (conv2d1 => "conv2d", trt => "tensorrt");
# say join " ", &expand_op_str($ARGV[0], \%defined_ops);

my $usage = <<EOF;
Usage: $0 [OPTION] [JSON_FILE(s)]
Generate expander code from expander description JSON.
Read the JSON string from standard input if JSON_FILE(s) are not given.
Print the output code to standard output if --dir and --root are omited.

Options:
  -h, --help              print this message
  -d, --dir=DIRECTORY     save operator defination file(s) in DIRECTORY
  -r, --root=ROOT         set project root directory; this option will save
                          operator defination file(s) in ROOT/src/op/auto, and
                          add operator declarations and such to
                          ROOT/src/arch/ln_arch_*.c
Author: Zhao Zhixu
EOF

my $INDENT_OFFSET = 4;

my $root = '';
my $dir = '';
GetOptions(
           'help' => sub {&exit_msg(0, $usage)},
           'dir=s' => \$dir,
           'root=s' => sub {$root = abs_path($_[1])},
          ) or &exit_msg(1, $usage);

my @json_files = @ARGV;
if (@json_files == 0) {
    my $json_text = join '', <STDIN>;
    my $json = &read_json_text($json_text);
    &gen_code($json);
} else {
    foreach my $file (@json_files) {
        my $json = &read_json($file);
        &gen_code($json);
    }
}

sub gen_code {
    my $json = shift;

    &err_exit("JSON needs an 'arch'") unless exists $json->{arch};
    &err_exit("JSON needs an 'name'") unless exists $json->{name};
    &err_exit("JSON needs an 'author'") unless exists $json->{author};
    &err_exit("JSON needs an 'ops'") unless exists $json->{ops};
    my $arch = $json->{arch};
    my $name = $json->{name};
    my $author = $json->{author};
    my $ops = $json->{ops};

    my @blocks = ();
    push @blocks, &gen_head_block($arch, $author);
    my @ep_funcs = ();
    foreach my $op (@$ops) {
        push @blocks, &gen_expander($op, \@ep_funcs);
    }
    push @blocks, &gen_init_ep_funcs(\@ep_funcs);
    push @blocks, &gen_overall_ep_func($json);

    my $code_str = join "\n", @blocks;
    if (not $dir and not $root) {
        print $code_str;
    }
    if ($dir) {
        my $dir_file = "${dir}/ln_expander_${name}.h";
        &backup_write($dir_file, $code_str);
    }
    if ($root) {
        my $src_file = "${root}/src/arch/auto/ln_expander_${name}.h";
        &backup_write($src_file, $code_str);
        my $arch_file = "${root}/src/arch/ln_arch_${arch}.c";
        &add_to_arch_file($arch_file, $arch, $name);
    }
}

sub gen_head_block {
    my $arch = shift;
    my $author = shift;
    my $head_block_tpl = <<EOF;
/*
 * Copyright (c) 2019 $author
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

 #include <ctype.h>
 #include <assert.h>

 #include "ln_arch.h"
 #include "ln_name.h"
 #include "ln_$arch.h"
EOF
}

sub gen_expander {
    my $op = shift;
    &err_exit("needs a 'optype'") unless exists $op->{optype};
    my $optype = $op->{optype};
    &err_exit("'${optype}' needs a 'rules'") unless exists $op->{rules};
    my $rules = $op->{rules};

    my @auto_vars = ();
    my $rules_code = "";
    foreach my $rule (@$rules) {
        &err_exit("needs a 'cond'") unless exists $rule->{cond};
        my %defined_ops = (self => $optype);
        my $cond_code = &gen_cond($rule->{cond}, \%defined_ops);

        my $body_code;
        if (exists $rule->{replace}) {
            foreach (@{$rule->{replace}}) {
                my ($type, $name) = split;
                $defined_ops{$name} = $type if defined $name;
            }
            $body_code = &gen_replace($rule->{replace}, $rule->{details},
                                      \@auto_vars, \%defined_ops);
        } elsif (exists $rule->{err}) {
            $body_code = "ln_msg_inter_error(\"ep_$optype(): $rule->{err}\");";
        } else {
            &err_exit("optype '$optype' needs a 'replace' or 'err'");
        }
        if (exists $rule->{warn}) {
            $body_code .= "\nln_msg_inter_warn(\"ep_$optype(): $rule->{warn}\");";
        }

        $body_code = &indent_block($INDENT_OFFSET, $body_code);
        if ($rule == $rules->[0]) {
            $rules_code .= <<EOF;
if ($cond_code) {
$body_code
}
EOF
        } else {
            $rules_code .= <<EOF;
else if ($cond_code) {
$body_code
}
EOF
        }
    }

    &make_defs_neat(\@auto_vars);
    my $auto_vars_code = join "\n", &indent_lines($INDENT_OFFSET, \@auto_vars);
    $rules_code = join "\n", &indent_block($INDENT_OFFSET, $rules_code);

    my $tpl = <<EOF;
static ln_list *ep_$optype(const ln_op *self, const ln_dfg *dfg, int *match)
{
$auto_vars_code

$rules_code
}
EOF
}

sub gen_cond {
    my $conds = shift;
    my $defined_ops = shift;

    my $cond_code;
    my $symbol_p = qr/[a-zA-Z0-9.\[\]()_"\\]+/;
    my @conds_replaced;
    foreach (@$conds) {
        my $cond = $_;
        while (/(($symbol_p)\s*(>|>=|<|<=|==|!=)\s*($symbol_p))/g) {
            my ($l_type, $l_code, $l_len) = &expand_op_str($2, $defined_ops, 0);
            my ($r_type, $r_code, $r_len) = &expand_op_str($4, $defined_ops, 0);
            my $operator = $3;
            my $type = &type_converse($l_type, $r_type);
            if (not defined $type) {
                &warn_msg("both operands' types are undefined in '$1', using literal string '$1'");
                $cond_code = $1;
                next;
            } elsif (not $type) {
                &err_exit("unmatched type '$l_type' and '$r_type' in '$1'");
            }

            &err_exit("unmatched operand lengths '$l_len' and '$r_len' in '$1'")
                if ((defined $l_len or defined $r_len) and $l_len != $r_len);

            $cond_code = &gen_comparator($3, $l_code, $r_code, $type, $l_len);
            substr($cond, index($cond, $1), length($1)) = $cond_code;
        }
        push @conds_replaced, "($cond)";
    }
    if (@$conds == 0) {
        "1";
    } else {
        join "||", @conds_replaced;
    }
}

sub gen_comparator {
    my $op = shift;
    my $lhs = shift;
    my $rhs = shift;
    my $type = shift;
    my $len = shift;

    my $code;
    given ($op) {
        when (/>|>=|<|<=|==|!=/) {
            given ($type) {
                when ("char *") {
                    $code = "(strcmp($lhs, $rhs) $op 0)"
                }
                when ("char **") {
                    $code = <<EOF;
({
    int result = 1;
    for (int i = 0; i < $len; i++) {
        if (!(strcmp(${lhs}[i], ${rhs}[i]) $op 0)) {
            result = 0;
            break;
        }
    }
    result;
})
EOF
                }
                when (/^(int|float|double|ln_bool) \*$/) {
                    $code = <<EOF;
({
    int result = 1;
    for (int i = 0; i < $len; i++) {
        if (!({lhs}[i] $op ${rhs}[i])) {
            result = 0;
            break;
        }
    }
    result;
})
EOF
                }
                when (/^(int|float|double|ln_bool|ln_mem_type)$/) {
                    $code = "($lhs $op $rhs)";
                }
                default {
                    $code = "($lhs $op $rhs)";
                    &warn_msg("'$type' uses default comparator '$code'");
                }
            }
        }
        default {
            &err_exit("'$op' is not a comparator operator");
        }
    }
    $code;
}

sub gen_replace {
    my $replace = shift;
    my $details = shift;
    my $auto_vars = shift;
    my $defined_ops = shift;

    my $optype = $defined_ops->{self};
    my $desc = &find_op_desc($optype);
    my $code;
    if (not defined $details) {
        &err_exit("need a 'details' to replace with multiple operators") if (@$replace != 1);
        my $rep_optype = (split ' ', $replace->[0])[0];
        my $rep_desc = &find_op_desc($rep_optype);
        # TODO: check validation
        $code = <<EOF;
ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table, self, "$rep_optype");
return ln_list_append(NULL, new_op);
EOF
        chomp $code;
        return $code;
    }

    my $symbol_p = qr/[a-zA-Z0-9.\[\]()_"\\]+/;
    my @blocks;
    foreach (@$details) {
        my $detail = $_;
        &check_details($_);
        /(($symbol_p)\s*(=)\s*($symbol_p))/;
        my ($l_type, $l_code, $l_len) = &expand_op_str($2, $defined_ops, 1);
        my ($r_type, $r_code, $r_len) = &expand_op_str($4, $defined_ops, 0);
        my $operator = $3;
        my $type = &type_converse($l_type, $r_type);
        if (not defined $type) {
            &warn_msg("both operands' types are undefined in '$1', using literal string '$1'");
            $cond_code = $1;
            next;
        } elsif (not $type) {
            $type = $r_type;
        }

        &err_exit("unmatched operand lengths '$l_len' and '$r_len' in '$1'")
            if ((defined $l_len or defined $r_len) and $l_len != $r_len);

        $cond_code = &gen_comparator($3, $l_code, $r_code, $type, $l_len);
        substr($cond, index($cond, $1), length($1)) = $cond_code;
        push @conds_replaced, "($cond)";
    }
}

sub check_details {
    my $details = shift;
    my $symbol_p = qr/[a-zA-Z0-9.\[\]()_"\\]+/;
    my %lhs_hash;
    foreach (@$details) {
        &err_exit("'details' can only contain assignments: '$_'")
            unless /(($symbol_p)\s*(=)\s*($symbol_p))/;
        my $lhs = $2;
        unless ($lhs =~ /^\w+\.(ins|outs|params)\[(\w+(\$\@)?)|(\w+\$\^\w*)|((\w*\$\^\w+))\]$/) {
            &err_exit("wrong syntax in the left-hand-side symbol of assignment '$_'");
        }
        if (exists $lhs_hash{$lhs}) {
            if ($lhs =~ /\$\@/) {
                delete $lhs_hash{$_} if /\$\^/ foreach keys %lhs_hash;
                next;
            }
            &err_exit("duplicated left-hand-side symbol of assignment '$_'");
        }
        $lhs_hash{$lhs} = 1;
    }
}

sub gen_init_ep_funcs {
}

sub gen_overall_ep_func {
}

sub type_converse {
    my $type1 = shift;
    my $type2 = shift;

    if (not defined $type1 and not defined $type2) {
        return undef;
    }
    if (defined $type1 and not defined $type2) {
        return $type1;
    }
    if (not defined $type1 and defined $type2) {
        return $type2;
    }
    if ($type1 eq $type2) {
        return $type1;
    }
    my %basic_types = (double=>1, float=>1, int=>1);
    if (not exists $basic_types{$type1} or not exists $basic_types{$type2}) {
        return 0;
    }
    if ($type1 eq "double" or $type2 eq "double") {
        return "double";
    } elsif ($type1 eq "float" or $type2 eq "float") {
        return "float"
    } else {
        return "int";
    }
    0;
}

sub add_to_arch_file {
    my $arch_file = shift;
    my $arch = shift;
    my $name = shift;

    my $declare = "extern ln_list *ln_expander_$name (const ln_op *op, const ln_dfg *dfg, int *match);";
    my $item = "ln_expander_$name,";

    copy($arch_file, "${arch_file}.bak")
        or die "Cannot backup file ${arch_file}: $!";
    open ARCH_FILE_BAK, '<', "${arch_file}.bak"
        or die "Cannot open ${arch_file}.bak: $!";
    open ARCH_FILE, '>', $arch_file
        or die "Cannot open ${arch_file}: $!";

    my $declared = 0;
    my $inited = 0;
    while (<ARCH_FILE_BAK>) {
        $declared = 1 if /$declare$/;
        s|/\* end of declare $arch expanders \*/|$declare\n/* end of declare $arch expanders */|
            unless $declared;
        $inited = 1 if /$item$/;
        s|/\* end of $arch expanders \*/|    $item\n/* end of $arch expanders */|
            unless $inited;
        print ARCH_FILE;
    }

    close ARCH_FILE;
    close ARCH_FILE_BAK;
}

sub expand_op_str {
    my $op_str = shift;
    my $defined_ops = shift;
    my $allow_variable = shift;
    my @fs = split /\.|(?=\[)/, $op_str;
    my ($type, $code, $len);
    unless (exists $defined_ops->{$fs[0]}) {
        if ($fs[0] =~ /\d+/) {
            $type = "int";
        } elsif ($fs[0] =~ /("(\\.|[^"\\])*")/) {
            $type = "char *";
        } elsif ($fs[0] =~ /([-+]?((\d*\.?\d+)|(\d+\.?\d*))([eE][-+]?\d+)?)/) {
            $type = "double";
        }
        return ($type, $op_str, $len);
    }
    my $optype = $defined_ops->{$fs[0]};

    my %member_path =
        (
         __self => ["ln_op *", "$fs[0]"],
         name => ["char *", "$fs[0]->op_arg->name"],
         optype => ["char *", "$fs[0]->op_arg->optype"],
         arch => ["char *", "$fs[0]->op_arg->arch"],
         tensor_table => ["char *", "$fs[0]->op_arg->tensor_table"],
         ins => {
                 __self => ["ln_list *", "$fs[0]->op_arg->tensors_in"],
                 len => ["int", "ln_list_length($fs[0]->op_arg->tensors_in)"],
                 "[]" => [\&expand_tensor, $allow_variable, "in", $optype,
                          $fs[0], @fs[2..@fs-1]],
                },
         outs => {
                  __self => ["ln_list *", "$fs[0]->op_arg->tensors_out"],
                  len => ["int", "ln_list_length($fs[0]->op_arg->tensors_out)"],
                  "[]" => [\&expand_tensor, $allow_variable, "out", $optype,
                           $fs[0], @fs[2..@fs-1]],
                 },
         params => {
                    __self => ["ln_list *", "$fs[0]->op_arg->params"],
                    len => ["int", "ln_list_length($fs[0]->op_arg->params)"],
                    "[]" => [\&expand_param, $allow_variable, $optype,
                             $fs[0], @fs[2..@fs-1]],
                   },
        );

    ($type, $code, $len) = &parse_member_path(\%member_path, @fs);
}

sub parse_member_path {
    my $path = shift;
    my @fs = @_;

    my ($type, $code, $len);
    my $ref = $path;
    my $index = -1;
    while (($index += 1) < @fs) {
        my $next_f = $fs[$index+1];
        if (ref $ref eq HASH) {
            if (ref $ref->{__self} eq ARRAY and ref $ref->{__self}->[0] eq CODE) {
                ($type, $code, $len) = &{$ref->{__self}->[0]}(@{$ref->{__self}}[1..@{$ref->{__self}}-1]);
            } else {
                ($type, $code, $len) = @{$ref->{__self}};
            }
            if ($next_f) {
                if ($next_f =~ /\[.+\]/) {
                    &err_unknown_field($index+1, @fs) unless exists $ref->{"[]"};
                    $ref = $ref->{"[]"};
                } elsif (exists $ref->{$next_f}) {
                    $ref = $ref->{$next_f};
                } else {
                    &err_unknown_field($index+1, @fs);
                }
            }
        } elsif (ref $ref eq ARRAY and ref $ref->[0] eq CODE) {
            ($type, $code, $len) = &{$ref->[0]}(@$ref[1..@$ref-1]);
        } else {
            ($type, $code, $len) = @$ref;
            &err_unknown_field($index+1, @fs) if ($next_f);
        }
    }

    ($type, $code, $len);
}

sub expand_tensor {
    my $allow_variable = shift;
    my $in_or_out = shift;
    my $optype = shift;
    my $opname = shift;
    my @fs = @_;
    my $arg_name = $fs[0] =~ s/\[(.+)\]/$1/r;

    my $tensors = "tensors_$in_or_out";
    my $op_desc = &find_op_desc($optype);
    my $found = 0;
    foreach (@{$op_desc->{$tensors}}) {
        $found = 1 if ($arg_name eq $_->{arg_name});
    }

    my ($type, $code, $len);
    if ($found) {
        my $entry = "ln_tensor_list_find_entry($opname->op_arg->$tensors, $opname->op_arg->tensor_table, \"$arg_name\")";
        my %tensor_member =
            (
             __self => ["char *", "$entry->name"],
             name => ["char *", "$entry->name"],
             owner => ["char *", "$entry->owner"],
             creater => ["char *", "$entry->creater"],
             tensor => ["tl_tensor *", "$entry->tensor"],
             offset => ["size_t", "$entry->offset"],
             isstatic => ["int", "$entry->isstatic"],
             mtype => ["ln_mem_type", "$entry->mtype"],
             dtype => ["tl_dtype", "$entry->tensor->dtype"],
             len => ["int", "$entry->tensor->len"],
             ndim => ["int", "$entry->tensor->ndim"],
             dims => {
                      __self => ["int *", "$entry->tensor->dims", "$entry->tensor->ndim"],
                      "[]" => [\&array_slice, "int", "$entry->tensor->dims", $fs[2]],
                     },
             data => ["void *", "$entry->tensor->data"],
             owner => ["struct tl_tensor *", "$entry->tensor->owner"],
             backend_data => ["void *", "$entry->tensor->backend_data"],
            );
        ($type, $code, $len) = &parse_member_path(\%tensor_member, @fs);
    } elsif ($allow_variable and $op_desc->{variable_length}) {
        # TODO: $@ can only exist in the last part of a string
        if ($arg_name =~ /\$\@$/) {
            $arg_name =~ s/\$\@$//;
            $type = "ln_tensor_list_entry *";
            $code = <<EOF;
({
    char arg_name[LN_MAX_NAME_LEN];
    last_index = ln_tensor_list_unique_arg_name($opname->op_arg->$tensors, arg_name, "$arg_name");
    $opname->op_arg->$tensors = ln_tensor_list_append($opname->op_arg->$tensors, arg_name, "");
    ln_tensor_list_find_by_arg_name($opname->op_arg->$tensors, arg_name);
})
EOF
        } elsif ($arg_name =~ /(.*)\$\^(.*)/) {
            my $arg_name1 = $1;
            my $arg_name2 = $2;
            $type = "ln_tensor_list_entry *";
            $code = <<EOF;
({
    char arg_name[LN_MAX_NAME_LEN];
    if (strlen("$arg_name1") + strlen("$arg_name2") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
        ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                           "$arg_name1", last_index, "$arg_name2", LN_MAX_NAME_LEN);
    snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "$arg_name1", last_index, "$arg_name2");
    $opname->op_arg->$tensors = ln_tensor_list_append($opname->op_arg->$tensors, arg_name, "");
    ln_tensor_list_find_by_arg_name($opname->op_arg->$tensors, arg_name);
})
EOF
        } else {
            $type = "ln_tensor_list_entry *";
            $code = <<EOF;
({
    $opname->op_arg->$tensors = ln_tensor_list_append($opname->op_arg->$tensors, "$arg_name", "");
    ln_tensor_list_find_by_arg_name($opname->op_arg->$tensors, "$arg_name");
})
EOF
        }
        &err_unknown_field(1, @fs) if $fs[1];
    } else {
        &util::err_exit("$opname($optype) doesn't have a '$arg_name' $tensors");
    }

    ($type, $code, $len);
}

sub expand_param {
    my $allow_variable = shift;
    my $optype = shift;
    my $opname = shift;
    my @fs = @_;
    my $arg_name = $fs[0] =~ s/\[(.+)\]/$1/r;

    my $op_desc = &find_op_desc($optype);
    my $found = 0;
    foreach (@{$op_desc->{params}}) {
        $found = 1 if ($arg_name eq $_->{arg_name});
    }

    my ($type, $code, $len);
    if ($found) {
        my $entry = "ln_param_list_find($opname->op_arg->params, \"$arg_name\")";
        my %param_member =
            (
             __self => [\&param_info, $optype, $entry, $arg_name],
             type => ["ln_param_type", "$entry->type"],
             array_len => ["int", "$entry->array_len"],
             len => ["int", "$entry->array_len"],
             value_double => ["double", "$entry->value_double"],
             value_float => ["float", "$entry->value_float"],
             value_int => ["int", "$entry->value_int"],
             value_bool => ["ln_bool", "$entry->value_bool"],
             value_string => ["char *", "$entry->value_string"],
             value_array_string => {
                                    __self => ["char **", "$entry->value_array_string",
                                               "$entry->array_len"],
                                    "[]" => [\&array_slice, "char *",
                                             "$entry->value_array_string", $fs[2]],
                                   },
             value_array_double => {
                                    __self => ["double *", "$entry->value_array_double",
                                               "$entry->array_len"],
                                    "[]" => [\&array_slice, "double",
                                             "$entry->value_array_double", $fs[2]],
                                   },
             value_array_float => {
                                   __self => ["float *", "$entry->value_array_float",
                                              "$entry->array_len"],
                                   "[]" => [\&array_slice, "float",
                                            "$entry->value_array_float", $fs[2]],
                                  },
             value_array_int => {
                                 __self => ["int *", "$entry->value_array_int",
                                            "$entry->array_len"],
                                 "[]" => [\&array_slice, "int",
                                          "$entry->value_array_int", $fs[2]],
                                },
             value_array_bool => {
                                  __self => ["ln_bool *", "$entry->value_array_bool",
                                             "$entry->array_len"],
                                  "[]" => [\&array_slice, "ln_bool",
                                           "$entry->value_array_bool", $fs[2]],
                                 },
             double => ["double", "$entry->value_double"],
             float => ["float", "$entry->value_float"],
             int => ["int", "$entry->value_int"],
             bool => ["ln_bool", "$entry->value_bool"],
             string => ["char *", "$entry->value_string"],
             array_string => {
                              __self => ["char **", "$entry->value_array_string",
                                         "$entry->array_len"],
                              "[]" => [\&array_slice, "char *",
                                       "$entry->value_array_string", $fs[2]],
                             },
             array_double => {
                              __self => ["double *", "$entry->value_array_double",
                                         "$entry->array_len"],
                              "[]" => [\&array_slice, "double",
                                       "$entry->value_array_double", $fs[2]],
                             },
             array_float => {
                             __self => ["float *", "$entry->value_array_float",
                                        "$entry->array_len"],
                             "[]" => [\&array_slice, "float",
                                      "$entry->value_array_float", $fs[2]],
                            },
             array_int => {
                           __self => ["int *", "$entry->value_array_int",
                                      "$entry->array_len"],
                           "[]" => [\&array_slice, "int",
                                    "$entry->value_array_int", $fs[2]],
                          },
             array_bool => {
                            __self => ["ln_bool *", "$entry->value_array_bool",
                                       "$entry->array_len"],
                            "[]" => [\&array_slice, "ln_bool",
                                     "$entry->value_array_bool", $fs[2]],
                           },
             "[]" => [\&param_slice, $optype, $entry, $arg_name, $fs[1]],
            );
        ($type, $code, $len) = &parse_member_path(\%param_member, @fs);
    } elsif ($allow_variable and $op_desc->{variable_length}) {
        # TODO: $@ can only exist in the last part of a string
        if ($arg_name =~ /\$\@$/) {
            $arg_name =~ s/\$\@$//;
            $type = "ln_param_entry *";
            $code = <<EOF;
({
    char arg_name[LN_MAX_NAME_LEN];
    last_index = ln_param_list_unique_arg_name($opname->op_arg->params, arg_name, "$arg_name");
    ln_param_entry *entry;
    entry = ln_param_entry_create(arg_name, LN_PARAM_INVALID);
    $opname->op_arg->params = ln_list_append($opname->op_arg->params, entry);
    ln_param_list_find($opname->op_arg->params, arg_name);
})
EOF
        } elsif ($arg_name =~ /(.*)\$\^(.*)/) {
            my $arg_name1 = $1;
            my $arg_name2 = $2;
            $type = "ln_param_entry *";
            $code = <<EOF;
({
    char arg_name[LN_MAX_NAME_LEN];
    if (strlen("$arg_name1") + strlen("$arg_name2") + ln_digit_num(last_index) >= LN_MAX_NAME_LEN)
        ln_msg_inter_error("name '%s%d%s' length exceeds LN_MAX_NAME_LEN = %d",
                           "$arg_name1", last_index, "$arg_name2", LN_MAX_NAME_LEN);
    snprintf(arg_name, LN_MAX_NAME_LEN, "%s%d%s", "$arg_name1", last_index, "$arg_name2");
    ln_param_entry *entry;
    entry = ln_param_entry_create(arg_name, LN_PARAM_INVALID);
    $opname->op_arg->params = ln_list_append($opname->op_arg->params, entry);
    ln_param_list_find($opname->op_arg->params, arg_name);
})
EOF
        } else {
            $type = "ln_param_entry *";
            $code = <<EOF;
({
    ln_param_entry *entry;
    entry = ln_param_entry_create("$arg_name", LN_PARAM_INVALID);
    $opname->op_arg->params = ln_list_append($opname->op_arg->params, entry);
    ln_param_list_find($opname->op_arg->params, "$arg_name");
})
EOF
        }
        &err_unknown_field(1, @fs) if $fs[1];
    } else {
        &util::err_exit("$opname($optype) doesn't have a '$arg_name' param");
    }

    ($type, $code, $len);
}

sub param_info {
    my $optype = shift;
    my $entry = shift;
    my $arg_name = shift;
    my $op_desc = &find_op_desc($optype);
    my ($element_type, $member);
    my ($type, $code, $len);
    foreach my $arg_name_desc (@{$op_desc->{params}}) {
        next unless $arg_name eq $arg_name_desc->{arg_name};
        given ($arg_name_desc->{ptype}) {
            when ("LN_PARAM_NULL") {
                $type = "ln_param_type";
                $member = "type";
            }
            when ("LN_PARAM_STRING") {
                $type = "char *";
                $member = "value_string";
            }
            when ("LN_PARAM_NUMBER") {
                $type = "$arg_name_desc->{realtype}";
                $member = "value_$arg_name_desc->{realtype}";
            }
            when ("LN_PARAM_BOOL") {
                $type = "ln_bool";
                $member = "value_bool";
            }
            when ("LN_PARAM_ARRAY_STRING") {
                $type = "char **";
                $len = "$entry->array_len";
                $member = "value_array_string";
            }
            when ("LN_PARAM_ARRAY_NUMBER") {
                $type = "$arg_name_desc->{realtype} *";
                $len = "$entry->array_len";
                $member = "value_array_$arg_name_desc->{realtype}";
            }
            when ("LN_PARAM_ARRAY_BOOL") {
                $type = "ln_bool *";
                $len = "$entry->array_len";
                $member = "value_array_bool";
            }
            default {
                &util::err_exit("unsupported ptype '$_' for optype '$optype''s param '$arg_name'");
            }
        }
    }
    $code = "$entry->$member";
    ($type, $code, $len);
}

sub param_slice {
    my $optype = shift;
    my $entry = shift;
    my $arg_name = shift;
    my $index_str = shift;
    my $op_desc = &find_op_desc($optype);
    my ($element_type, $member);
    my ($type, $code);
    foreach my $arg_name_desc (@{$op_desc->{params}}) {
        next unless $arg_name eq $arg_name_desc->{arg_name};
        given ($arg_name_desc->{ptype}) {
            when ("LN_PARAM_ARRAY_STRING") {
                $type = "char **";
                $element_type = "char *";
                $member = "value_array_string";
            }
            when ("LN_PARAM_ARRAY_NUMBER") {
                $type = "$arg_name_desc->{realtype} *";
                $element_type = "$arg_name_desc->{realtype}";
                $member = "value_array_$arg_name_desc->{realtype}";
            }
            when ("LN_PARAM_ARRAY_BOOL") {
                $type = "ln_bool *";
                $element_type = "ln_bool";
                $member = "value_array_bool";
            }
            default {
                &util::err_exit("unsupported '[]' operator for optype '$optype''s param '$arg_name'");
            }
        }
    }
    $code = "$entry->$member";
    ($type, $code) = &array_slice($element_type, $code, $index_str);
}

sub array_slice {
    my $element_type = shift;
    my $initial_code = shift;
    my $index_str = shift;
    $index_str =~ s/\[(.+)\]/$1/;
    my @indexes = split /\s*,\s*/, $index_str;

    my ($type, $code);
    if (@indexes == 1) {
        ($type, $code) = ($element_type, "${initial_code}[$indexes[0]]");
    } else {
        $type = "$element_type *";
        my @array;
        foreach (@indexes) {
            push @array, "${initial_code}[$_]";
        }
        my $array_str = join ', ', @array;
        $code = "(${element_type}[]){$array_str}";
    }

    ($type, $code);
}

sub find_op_desc {
    my $optype = shift;
    my $op = $global_ops{$optype};
    unless ($op) {
        my $opdir = abs_path(dirname(__FILE__))."/../protos/op";
        my @possible_files = &possible_op_files($optype);
        foreach (@possible_files) {
            my $file = "$opdir/$_";
            next unless -e $file;
            &read_ops_json($file, \%global_ops);
            $op = $global_ops{$optype} if exists $global_ops{$optype};
        }
        unless ($op) {
            &util::err_exit("Cannot find the description JSON for optype '$optype'");
        }
    }
    $op;
}

sub possible_op_files {
    my $optype = shift;
    my @names = ();
    my @words = split '_', $optype;
    if (@words == 1) {
        push @names, $optype.'.json';
    } else {
        push @names, (join '_', @words[0..$#words-1]).'.json';
        push @names, $optype.'.json';
    }
    @names;
}

sub read_json_text {
    my $json_text = shift;
    $json_text = easyjson::easy_to_json($json_text);
    my $json_obj = JSON->new->relaxed();
    my $json = $json_obj->decode($json_text);
}

sub read_json {
    my $file = shift;
    open my $fh, '<', $file or die "Cannot open $file: $!";
    my $text = join '', <$fh>;
    close $fh;
    &read_json_text($text);
}

sub read_ops_json {
    my $file = shift;
    my $hash = shift;
    my $json = &read_json($file);
    if (exists $json->{ops}) {
        foreach my $op (@{$json->{ops}}) {
            $hash->{$op->{optype}} = $op;
        }
    } elsif (exists $json->{optype}) {
        $hash->{$json->{optype}} = $json;
    } else {
        &util::err_exit("JSON file $file doesn't contain an 'ops' or 'optype' field");
    }
}

sub err_unknown_field {
    my $index = shift;
    my @fields = @_;
    my $prefix = join '.', @fields[0..$index-1];
    my $subfix = $fields[$index];
    &util::err_exit("$prefix doesn't have a '$subfix' field");
}

sub err_unknown_last_field {
    my @fields = @_;
    &err_unknown_field($#fields, @fields);
}

1;

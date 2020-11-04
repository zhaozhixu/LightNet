#! /usr/bin/env perl

package graphlang;

use 5.014;
use warnings;
use strict;
use JSON;
use File::Copy;
use Cwd 'abs_path';
use Getopt::Long;
use File::Basename;
use lib abs_path(dirname(__FILE__));
use util;
use easyjson;
no warnings 'experimental::smartmatch';

my %global_ops;
# &read_ops_from_json_file("protos/op/arange.json", \%global_ops);
# &read_ops_from_json_file("protos/op/conv2d.json", \%global_ops);
# while (my ($key, $value) = each %global_ops) {
#     say "$key: $value->{optype}"
# }
# my $op = &find_op_desc("conv2d_cuda1");
# say $op->{author};
# print join "\n", &possible_op_files("conv2d_cuda1");
# if (my $op = &find_op_desc("reshape")) {
#     say $op->{optype};
# }
my %defined_ops = (conv2d1 => "conv2d");
my $op_str = "$(conv2d1.group = conv2d1.size[0])";
my ($type, $expanded) = &expand_op_str($ARGV[0], \%defined_ops);
say "$type $expanded";

my @global_vars = ("int last_index");

sub expand_op_str {
    my $op_str = shift;
    my $defined_ops = shift;

    my %directives;
    $op_str = &get_and_remove_directives($op_str, \%directives);
    my @fields = split /\./, $op_str;
    my $name = $fields[0];
    unless (exists $defined_ops->{$name}) {
        # TODO
    }

    my $optype = $defined_ops->{$name};
    my $type = "ln_op *";
    my $expanded = "$fields[0]";
    return ($type, "($expanded)") unless $fields[1];

    given ($fields[1]) {
        when ("name") {$type = "char *"; continue}
        when ("optype") {$type = "char *"; continue}
        when ("arch") {$type = "char *"; continue}
        when ("tensor_table") {$type = "ln_hash *"; continue;}
        when (/name|optype|arch|tensor_table/) {
            $expanded .= "->op_arg->$_";
            return ($type, "($expanded)") unless $fields[2];
            &err_unknown_last_field(@fields);
        }
        when ("tensors_in") {$type = "ln_list *"; continue}
        when ("tensors_out") {$type = "ln_list *"; continue}
        when ("params") {$type = "ln_list *"; continue}
        when (/tensors_in|tensors_out|params/) {
            $expanded .= "->op_arg->$_";
            return ($type, "($expanded)") unless $fields[2];
            given ($fields[2]) {
                when ("len") {
                    $type = "int";
                    $expanded = "ln_list_length($expanded)";
                    return ($type, "($expanded)") unless $fields[3];
                    &err_unknown_last_field(@fields);
                }
                default {
                    &err_unknown_last_field(@fields);
                }
            }
        }
        when (&field_is_tensor($_, $optype)) {
            return &expand_tensor(@fields);
        }
        when (&field_is_param($_, $optype)) {
            return &expand_param($optype, @fields);
        }
        when (exists $directives{"add-in"}) {
            return &expand_add_in(@fields);
        }
        when (exists $directives{"add-out"}) {
            return &expand_add_out(@fields);
        }
        when (exists $directives{"add-param"}) {
            return &expand_add_param($optype, $directives{"add-param"}, @fields);
        }
        default {
            &err_unknown_last_field(@fields);
        }
    }
}

sub expand_tensor {
    my @fields = @_;
    my $type = "char *";
    my $expanded = "ln_op_find_tensor_list_entry($fields[0], $fields[1])->name";
    return ($type, "($expanded)") unless $fields[2];
    $expanded = "ln_op_find_tensor_entry($fields[0], $fields[1])";
    given ($fields[2]) {
        when ("name") {$type = "char *"; continue}
        when ("owner") {$type = "char *"; continue}
        when ("creater") {$type = "char *"; continue}
        when ("tensor") {$type = "tl_tensor *"; continue}
        when ("offset") {$type = "size_t"; continue}
        when ("isstatic") {$type = "int"; continue}
        when ("mtype") {$type = "ln_mem_type"; continue}
        when (/name|owner|creater|tensor|offset|isstatic|mtype/) {
            $expanded = "$expanded->$_";
            return ($type, "($expanded)") unless $fields[3];
            &err_unknown_last_field(@fields);
        }
        when ("dtype") {$type = "tl_dtype"; continue}
        when ("len") {$type = "int"; continue}
        when ("ndim") {$type = "int"; continue}
        when ("dims") {$type = "int *"; continue}
        when ("data") {$type = "void *"; continue}
        when ("owner") {$type = "struct tl_tensor *"; continue}
        when ("backend_data") {$type = "void *"; continue}
        when (/dtype|len|ndim|dims|data|owner|backend_data/) {
            $expanded = "$expanded->tensor->$_";
            return ($type, "($expanded)") unless $fields[3];
            &err_unknown_last_field(@fields);
        }
        default {
            &err_unknown_last_field(@fields);
        }
    }
}

sub expand_param {
    my $optype = shift;
    my @fields = @_;
    my ($type, $member) = &param_type_and_member($fields[1], $optype);
    my $expanded = "ln_param_list_find($fields[0]->op_arg->params, $fields[1])->$member";
    return ($type, "($expanded)") unless $fields[2];
    $expanded = "ln_param_list_find($fields[0]->op_arg->params, $fields[1])";
    given ($fields[2]) {
        when ("type") {
            $type = "ln_param_type";
            $expanded = "$expanded->type";
            return ($type, "($expanded)") unless $fields[3];
            &err_unknown_last_field(@fields);
        }
        when (/array_len|len/) {
            $type = "int";
            $expanded = "$expanded->array_len";
            return ($type, "($expanded)") unless $fields[3];
            &err_unknown_last_field(@fields);
        }
        default {
            &err_unknown_last_field(@fields);
        }
    }
}

sub expand_add_in {
    my @fields = @_;
    my $type = "char *";
    my $expanded;
    # TODO: $@ can only exist in the last part of a string
    if (/\$\@$/) {
        my $field = $fields[1] =~ s/\$\@$//r;
        $expanded = <<EOF;
({
    ln_list *ins = $fields[0]->op_arg->tensors_in;
    char arg_name[LN_MAX_NAME_LEN];
    last_index = ln_tensor_list_sprint_arg_name(ins, arg_name, $field);
    $fields[0]->op_arg->tensors_in = ln_tensor_list_append(ins, arg_name, "");
    ln_op_find_tensor_list_entry($fields[0], arg_name)->name;
})
EOF
    } else {
        $expanded = <<EOF;
({
    ln_list *ins = $fields[0]->op_arg->tensors_in;
    $fields[0]->op_arg->tensors_in = ln_tensor_list_append(ins, $fields[1], "");
    ln_op_find_tensor_list_entry($fields[0], arg_name)->name;
})
EOF
    }
    return ($type, $expanded) unless $fields[2];
    &err_unknown_last_field(@fields);
}

sub expand_add_out {
    my @fields = @_;
    my $type = "char *";
    my $expanded;
    # TODO: $@ can only exist in the last part of a string
    if (/\$\@$/) {
        my $field = $fields[1] =~ s/\$\@$//r;
        $expanded = <<EOF;
({
    ln_list *outs = $fields[0]->op_arg->tensors_out;
    char arg_name[LN_MAX_NAME_LEN];
    last_index = ln_tensor_list_sprint_arg_name(outs, arg_name, $field);
    $fields[0]->op_arg->tensors_out = ln_tensor_list_append(outs, arg_name, "");
    ln_op_find_tensor_list_entry($fields[0], arg_name)->name;
})
EOF
    } else {
        $expanded = <<EOF;
({
    ln_list *outs = $fields[0]->op_arg->tensors_out;
    $fields[0]->op_arg->tensors_out = ln_tensor_list_append(outs, $fields[1], "");
    ln_op_find_tensor_list_entry($fields[0], arg_name)->name;
})
EOF
    }
    return ($type, $expanded) unless $fields[2];
    &err_unknown_last_field(@fields);
}

sub expand_add_param {
    my $optype = shift;
    my $arg_str = shift;
    my @fields = @_;

    my ($type, $expanded, $member);
    if ($arg_str) {
        my @args = split /\s*,\s*/, $arg_str;
        &util::err_exit("wrong number of arguments of directive 'add-param'")
            if @args < 1 or @args > 2;
        $type = $args[0];
        $member = &param_type_to_member($type);
    }
    # TODO: $@ can only exist in the last part of a string
    if (/\$\@$/) {
        my $field = $fields[1] =~ s/\$\@$//r;
        $expanded = <<EOF;
({
    ln_list *params = $fields[0]->op_arg->params;
    char arg_name[LN_MAX_NAME_LEN];
    last_index = ln_param_list_sprint_arg_name(params, arg_name, $field);
    $fields[0]->op_arg->params = ln_tensor_list_append(params, arg_name, "");
    ln_op_find_tensor_list_entry($fields[0], arg_name)->name;
})
EOF
    } else {
        $expanded = <<EOF;
({
    ln_list *params = $fields[0]->op_arg->params;
    $fields[0]->op_arg->params = ln_tensor_list_append(params, $fields[1], "");
    ln_op_find_tensor_list_entry($fields[0], arg_name)->name;
})
EOF
    }
    return ($type, $expanded) unless $fields[2];
    &err_unknown_last_field(@fields);
}

sub param_type_to_member {
    my $type = shift;
    my $member;
    given ($type) {
        when ("ln_param_type") {$member = "type"}
        when ("char *") {$member = "value_string"}
        when ("double") {$member = "value_doubl"}
        when ("float") {$member = "value_float"}
        when ("int") {$member = "value_int"}
        when ("ln_bool") {$member = "value_bool"}
        when ("char **") {$member = "value_array_string"}
        when ("double *") {$member = "value_array_double"}
        when ("float *") {$member = "value_array_float"}
        when ("int *") {$member = "value_array_int"}
        when ("ln_bool *") {$member = "value_array_bool"}
        default {
            &util::err_exit("unsupported param type '$type'");
        }
    }
    $member;
}

sub get_and_remove_directives {
    my $op_str = shift;
    my $hash = shift;
    if ($op_str =~ /^\$\((?<name>[-a-zA-Z0-9_]+)(\((?<arg>.+)\))?\s+(?<op>.+)\)/) {
        if (exists $+{arg}) {
            $hash->{$+{name}} = $+{arg};
        } else {
            $hash->{$+{name}} = "";
        }
        $op_str = $+{op};
    }
    $op_str;
}

sub field_is_tensor {
    my $field = shift;
    my $optype = shift;
    my $op_desc = &find_op_desc($optype);
    foreach (@{$op_desc->{tensors_in}}) {
        return 1 if $field eq $_->{arg_name};
    }
    foreach (@{$op_desc->{tensors_out}}) {
        return 1 if $field eq $_->{arg_name};
    }
    return 0;
}

sub field_is_param {
    my $field = shift;
    my $optype = shift;
    my $op_desc = &find_op_desc($optype);
    foreach (@{$op_desc->{params}}) {
        return 1 if $field eq $_->{arg_name};
    }
    return 0;
}

sub param_type_and_member {
    my $param = shift;
    my $optype = shift;
    my $op_desc = &find_op_desc($optype);
    my ($type, $member);
    foreach my $param_desc (@{$op_desc->{params}}) {
        next unless $param eq $param_desc->{arg_name};
        given ($param_desc->{ptype}) {
            when ("LN_PARAM_NULL") {
                $type = "ln_param_type";
                $member = "type";
            }
            when ("LN_PARAM_STRING") {
                $type = "char *";
                $member = "value_string";
            }
            when ("LN_PARAM_NUMBER") {
                $type = $param_desc->{realtype};
                $member = "value_$param_desc->{realtype}";
            }
            when ("LN_PARAM_BOOL") {
                $type = "ln_bool";
                $member = "value_bool";
            }
            when ("LN_PARAM_ARRAY_STRING") {
                $type = "char **";
                $member = "value_array_string";
            }
            when ("LN_PARAM_ARRAY_NUMBER") {
                $type = "$param_desc->{realtype} *";
                $member = "value_array_$param_desc->{realtype}";
            }
            when ("LN_PARAM_ARRAY_BOOL") {
                $type = "ln_bool *";
                $member = "value_array_bool";
            }
            default {
                &util::err_exit("unsupported ptype '$param_desc->{ptype}' for optype '$optype''s param '$param'");
            }
        }
    }
    ($type, $member);
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
            &read_ops_from_json_file($file, \%global_ops);
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

sub read_json {
    my $file = shift;
    open my $fh, '<', $file or die "Cannot open $file: $!";
    my $text = join '', <$fh>;
    close $fh;
    my $json_text = easyjson::easy_to_json($text);
    my $json_obj = JSON->new->relaxed();
    my $json = $json_obj->decode($json_text);
}

sub read_ops_from_json_file {
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

sub err_unknown_last_field {
    my @fields = @_;
    my $prefix = join '.', @fields[0..$#fields-1];
    my $suffix = $fields[-1];
    &util::err_exit("$prefix doesn't have a '$suffix' field");
}

1;

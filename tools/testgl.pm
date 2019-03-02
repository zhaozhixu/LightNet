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
my %defined_ops = (conv2d1 => "conv2d", trt => "tensorrt");
say join " ", &expand_op_str(\%defined_ops, $ARGV[0]);
my @global_vars = ("int last_index");

sub expand_op_str {
    my $defined_ops = shift;
    my $op_str = shift;
    my @fs = split /\.|(?=\[)/, $op_str;
    unless (exists $defined_ops->{$fs[0]}) {
        &util::err_exit("undefined operator '$fs[0]'");
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
                 "[]" => [\&expand_tensor, "in", $optype, $fs[0], @fs[2..@fs-1]],
                },
         outs => {
                  __self => ["ln_list *", "$fs[0]->op_arg->tensors_out"],
                  len => ["int", "ln_list_length($fs[0]->op_arg->tensors_out)"],
                  "[]" => [\&expand_tensor, "out", $optype, $fs[0], @fs[2..@fs-1]],
                 },
         params => {
                    __self => ["ln_list *", "$fs[0]->op_arg->params"],
                    len => ["int", "ln_list_length($fs[0]->op_arg->params)"],
                    "[]" => [\&expand_param, $optype, $fs[0], @fs[2..@fs-1]],
                   },
        );

    my ($type, $code) = &parse_member_path(\%member_path, @fs);
}

sub parse_member_path {
    my $path = shift;
    my @fs = @_;

    my ($type, $code);
    my $ref = $path;
    my $index = -1;
    while (($index += 1) < @fs) {
        my $next_f = $fs[$index+1];
        if (ref $ref eq HASH) {
            ($type, $code) = @{$ref->{__self}};
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
            ($type, $code) = &{$ref->[0]}(@$ref[1..@$ref-1]);
        } else {
            ($type, $code) = @$ref;
            &err_unknown_field($index+1, @fs) if ($next_f);
        }
    }

    ($type, $code);
}

sub expand_tensor {
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

    my ($type, $code);
    if ($found) {
        my $entry = "ln_tensor_list_find_entry($opname->op_arg->$tensors, $opname->op_arg->tensor_table, \"$arg_name\")";
        my %tensor_member =
            (
             __self => ["ln_tensor_entry *", "$entry"],
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
                      __self => ["int *", "$entry->tensor->dims"],
                      "[]" => [\&array_slice, "int", "$entry->tensor->dims", $fs[2]],
                     },
             data => ["void *", "$entry->tensor->data"],
             owner => ["struct tl_tensor *", "$entry->tensor->owner"],
             backend_data => ["void *", "$entry->tensor->backend_data"],
            );
        ($type, $code) = &parse_member_path(\%tensor_member, @fs);
    } elsif ($op_desc->{variable_length}) {
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

    ($type, $code);
}

sub expand_param {
    my $optype = shift;
    my $opname = shift;
    my @fs = @_;
    my $arg_name = $fs[0] =~ s/\[(.+)\]/$1/r;

    my $op_desc = &find_op_desc($optype);
    my $found = 0;
    foreach (@{$op_desc->{params}}) {
        $found = 1 if ($arg_name eq $_->{arg_name});
    }

    my ($type, $code);
    if ($found) {
        my $entry = "ln_param_list_find($opname->op_arg->params, \"$arg_name\")";
        my %param_member =
            (
             __self => ["ln_param_entry *", "$entry"],
             type => ["ln_param_type", "$entry->type"],
             array_len => ["int", "$entry->array_len"],
             len => ["int", "$entry->array_len"],
             value_double => ["double", "$entry->value_double"],
             value_float => ["float", "$entry->value_float"],
             value_int => ["int", "$entry->value_int"],
             value_bool => ["ln_bool", "$entry->value_bool"],
             value_string => ["char *", "$entry->value_string"],
             value_array_string => {
                                    __self => ["char **", "$entry->value_array_string"],
                                    "[]" => [\&array_slice, "char *",
                                             "$entry->value_array_string", $fs[2]],
                                   },
             value_array_double => {
                                    __self => ["double *", "$entry->value_array_double"],
                                    "[]" => [\&array_slice, "double",
                                             "$entry->value_array_double", $fs[2]],
                                   },
             value_array_float => {
                                   __self => ["float *", "$entry->value_array_float"],
                                   "[]" => [\&array_slice, "float",
                                            "$entry->value_array_float", $fs[2]],
                                  },
             value_array_int => {
                                 __self => ["int *", "$entry->value_array_int"],
                                 "[]" => [\&array_slice, "int",
                                          "$entry->value_array_int", $fs[2]],
                                },
             value_array_bool => {
                                  __self => ["ln_bool *", "$entry->value_array_bool"],
                                  "[]" => [\&array_slice, "ln_bool",
                                           "$entry->value_array_bool", $fs[2]],
                                 },
             "[]" => [\&param_slice, $optype, $entry, $arg_name, $fs[1]],
            );
        ($type, $code) = &parse_member_path(\%param_member, @fs);
    } elsif ($op_desc->{variable_length}) {
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

    ($type, $code);
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

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

sub expand_op_str {
    my $defined_ops = shift;
    my $op_str = shift;
    my @fs = split /\.|(?=\[)/, $op_str;
    unless (exists $defined_ops->{$fs[0]}) {
        # TODO:
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
                  "[]" => [\&expand_outs],
                 },
         params => {
                    __self => ["ln_list *", "$fs[0]->op_arg->params"],
                    len => ["int", "ln_list_length($fs[0]->op_arg->params)"],
                    "[]" => [\&expand_params],
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
    $fs[0] =~ s/\[(.+)\]/$1/;

    my $tensors = "tensors_$in_or_out";
    my $op_desc = &find_op_desc($optype);
    my $found = 0;
    foreach (@{$op_desc->{$tensors}}) {
        $found = 1 if ($fs[0] eq $_->{arg_name});
    }

    my ($type, $code);
    if ($found) {
        my $entry = "ln_tensor_list_find_entry($opname->op_arg->$tensors, $opname->op_arg->tensor_table, \"$fs[0]\")";
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
                      "[]" => [\&array_slice, "int"]
                     },
             data => ["void *", "$entry->tensor->data"],
             owner => ["struct tl_tensor *", "$entry->tensor->owner"],
             backend_data => ["void *", "$entry->tensor->backend_data"],
            );
        ($type, $code) = &parse_member_path(\%tensor_member, @fs);
    } elsif ($op_desc->{variable_length}) {
        $type = "ln_tensor_list_entry *";
        $code = <<EOF;
({
    ln_list *tensors = $opname->op_arg->$tensors;
    $opname->op_arg->$tensors = ln_tensor_list_append(tensors, \"$fs[0]\", "");
    ln_op_find_tensor_list_entry($opname, arg_name);
})
EOF
    } else {
        &util::err_exit("$opname($optype) doesn't have a '$fs[0]' $tensors");
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

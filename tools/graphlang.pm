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
use util qw(err_exit warn_msg exit_msg);
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
if (my $op = &find_op_desc("reshape")) {
    say $op->{optype};
}

sub expand_op_str {
    my $op_str = shift;
    my $defined_ops = shift;

    my %directives;
    $op_str = &get_and_remove_directives($op_str, \%directives);
    my @fields = split '.', $op_str;
    my $name = $fields[0];
    unless (exists $defined_ops->{$name}) {
        # TODO
    }

    my $optype = $defined_ops->{$name};
    my $type = "ln_op *"
    my $expanded = "$fields[0]";
    return ($type, "($expanded)") unless $fields[1];

    given ($fields[1]) {
        when ("name") {$type = "char *"; continue}
        when ("optype") {$type = "char *"; continue}
        when ("arch") {$type = "char *; "continue}
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
            $type = "char *";
            $expanded = "ln_op_find_tensor_list_entry($name, $_)->name";
            return ($type, "($expanded)") unless $fields[3];
            $expanded = "ln_op_find_tensor_entry($name, $_)";
            given ($fields[3]) {
                when ("name") {$type = "char *"; continue}
                when ("owner") {$type = "char *"; continue}
                when ("creater") {$type = "char *"; continue}
                when ("tensor") {$type = "tl_tensor *"; continue}
                when ("offset") {$type = "size_t"; continue}
                when ("isstatic") {$type = "int"; continue}
                when ("mtype") {$type = "ln_mem_type"; continue}
                when (/name|owner|creater|tensor|offset|isstatic|mtype/) {
                    $expanded .= "->$_";
                    return ($type, "($expanded)") unless $fields[4];
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
                    $expanded .= "->tensor->$_";
                    return ($type, "($expanded)") unless $fields[4];
                    &err_unknown_last_field(@fields);
                }
                default {
                    &err_unknown_last_field(@fields);
                }
            }
        }
        when (&field_is_param($_, $optype)) {
            my $member;
            ($type, $member) = &param_type_and_member($_, $optype);
            $expanded = "ln_param_list_find($name->op_arg->params, $_)->$member";
            return ($type, "($expanded)") unless $fields[3];
            $expanded = "ln_param_list_find($name->op_arg->params, $_)";
            given ($fields[3]) {
                when ("type") {
                    $type = "ln_param_type";
                    $expanded .= "->type";
                    return ($type, "($expanded)") unless $fields[4];
                    &err_unknown_last_field(@fields);
                }
                when (/array_len|len/) {
                    $type = "int";
                    $expanded .= "->array_len";
                    return ($type, "($expanded)") unless $fields[4];
                    &err_unknown_last_field(@fields);
                }
                default {
                    &err_unknown_last_field(@fields);
                }
            }
        }
        when ($directives{auto-in}) {

        }
        default {
            &err_unknown_last_field(@fields);
        }
    }
}

sub get_and_remove_directives {
    my $op_str = shift;
    my $hash = shift;
    if ($op_str =~ /^\$\(([-a-zA-Z0-9_]+)\s+(.+)\)/) {
        $hash->{$1} = 1;
        $op_str = $2;
    }
    $op_str;
}

sub field_is_tensor {
    my $field = shift;
    my $optype = shift;
    my $op_desc = &find_op_desc($optype);
    foreach (@{$op_desc->{tesors_in}}) {
        return 1 if $field eq $_->{arg_name};
    }
    foreach (@{$op_desc->{tesors_out}}) {
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
    foreach (@{$op_desc->{params}}) {
        next unless $param eq $_->{arg_name};
        given ($_->{ptype}) {
            when ("LN_PARAM_NULL") {
                $type = "ln_param_type";
                $member = "type";
            }
            when ("LN_PARAM_STRING") {
                $type = "char *";
                $member = "value_string";
            }
            when ("LN_PARAM_NUMBER") {
                $type = $_->{realtype};
                $member = "value_$_->{realtype}";
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
                $type = "$_->{realtype} *";
                $member = "value_array_$_->{realtype}";
            }
            when ("LN_PARAM_ARRAY_BOOL") {
                $type = "ln_bool *";
                $member = "value_array_bool";
            }
            default {
                &err_exit("unsupported ptype '$_->{ptype}' for optype '$optype''s param '$param'");
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
            &err_exit("Cannot find the description JSON for optype '$optype'");
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
        &err_exit("JSON file $file doesn't contain an 'ops' or 'optype' field");
    }
}

sub err_unknown_last_field {
    my @fields = @_;
    my $prefix = join '.', @fields[0..$#fields-1];
    my $subfix = $fields[-1];
    &err_exit("$prefix doesn't have a '$subfix' field");
}

1;

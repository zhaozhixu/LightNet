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
# my $op = &find_op("conv2d_cuda1");
# say $op->{author};
# print join "\n", &possible_op_files("conv2d_cuda1");
if (my $op = &find_op("reshape")) {
    say $op->{optype};
}

sub find_op {
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
    }
    return $op;
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

1;

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
no warnings 'experimental::smartmatch';

say join " ", &expand_op($ARGV[0]);

sub expand_op {
    my $op_str = shift;
    my @fs = split /\./, $op_str;

    my %member_path =
        (
         _nothing => ["ln_op *", "$fs[0]"],
         name => ["char *", "$fs[0]->op_arg->name"],
         optype => ["char *", "$fs[0]->op_arg->optype"],
         arch => ["char *", "$fs[0]->op_arg->arch"],
         tensor_table => ["char *", "$fs[0]->op_arg->tensor_table"],
         ins => {
                 _nothing => ["ln_list *", "$fs[0]->op_arg->tensors_in"],
                 len => ["int", "ln_list_length($fs[0]->op_arg->tensors_in)"],
                 _square => [\&expand_ins],
                },
         outs => {
                  _nothing => ["ln_list *", "$fs[0]->op_arg->tensors_out"],
                  len => ["int", "ln_list_length($fs[0]->op_arg->tensors_out)"],
                  _square => [\&expand_outs],
                 },
         params => {
                    _nothing => ["ln_list *", "$fs[0]->op_arg->params"],
                    len => ["int", "ln_list_length($fs[0]->op_arg->params)"],
                    _square => [\&expand_params],
                   },
        );

    my ($type, $code);
    my $index = 0;
    my $ref = \%member_path;
    while ($index < @fs) {
        $index += 1;
        if (reftype $ref eq ref {}) {
            ($type, $code) = @{$ref->{_nothing}};
            $ref = $ref->{$fs[$index]} if ($fs[$index]);
        } else {
            ($type, $code) = @$ref;
        }
    }

    ($type, $code);
}

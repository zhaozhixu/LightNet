#! /usr/bin/env perl

use 5.014;
use warnings;
use strict;
use JSON;
no warnings 'experimental::smartmatch';

my $usage =<<EOF;
Usage: $0 [OPTION...] INFILE
Generate JSON-format IR code from INFILE in
EOF

my @ops = ();

my %op = (
          'name' => $name,
          'optype' => $optype,
          'tensors_in' => \@tensors_in,
          'tensors_out' => \@tensors_out,
          'params' => \@params,
         );
}

my $name_p = qr/([a-zA-Z_][a-zA-Z0-9_]*)/;
my $num_p = qr/([-+]?((\d*\.?\d+)|(\d+\.?\d*))([eE][-+]?\d+)?)/;
my $str_p = qr/("(\\.|[^"\\])*")/;
my $bool_p = qr/(true|false)/;
my $type_p = qr/($num_p|$bool_p|$str_p|null)/;
my $arr_p = qr/(\[(\s*$type_p\s*,)*\s*$type_p\s*,?\s*\])/;
my $value_p = qr/($type_p|$arr_p|$name_p)/;
my $kv_p = qr/($name_p\s*:\s*$value_p)/;

sub gen_op {
    my $state = shift;
    my $optype;
    my $ins_str;
    my $outs_str;
    my $params_str;

    my $args_p = qr/((\s*$kv_p\s*,)*\s*$kv_p\s*,?)/;
    my $arg_list_p = qr/((?<ins>$args_p)\s*\|\s*(?<outs>$args_p)\s*\|\s*(?<params>$args_p))/;
    my $state_p = qr/((?<optype>$name_p)\s*\(\s*$arg_list_p\s*\))/;

    if ($state =~ /^$state_p$/) {
        $optype = $+{optype};
        $ins_str = $+{ins};
        $outs_str = $+{outs};
        $params_str = $+{params};
    } else {
        &err_exit("wrong syntax in\n    $state");
    }

    my %op = (
              'optype' => $optype,
              'tensors_in' => &gen_tensors($ins_str),
              'tensors_out' => &gen_tensors($outs_str),
              'params' => &gen_params($params_str),
             );
}

sub gen_tensors {
    my $tensors_str = shift;
    my @tensors = ();
    foreach (split ',', $tensors_str) {
        my ($arg_name, $tname) = map {s/^\s+//;s/\s+$//;$_} split ':';
        &err_exit("wrong tensor name format in '$tname'")
          unless $tname =~ /^$name_p$/;
        my %tensor = (
                      'arg_name' => $arg_name,
                      'tname' => $tname,
                     );
        push @tensors, \%tensor;
    }
    \@tensors;
}

sub gen_params {
    my $params_str = shift;
    my @params = ();
    foreach (split ',', $tensors_str) {
        my ($arg_name, $value) = map {s/^\s+//;s/\s+$//;$_} split ':';
        given ($value) {
            when (/^$num_p$/) {
                $value += 0;
            }
            when (/^$bool_p$/) {
                if ($value eq "true") {
                    $value = \1;
                } else {
                    $value = \0;
                }
            }
            when (/^$str_p$/) {
                $value =~ s/^"//;
                $value =~ s/"$//;
            }
            when (/^null$/) {
                $value = undef;
            }
            when (/^$arr_p$/) {
                $value = &gen_array($value);
            }
            when (/^$name_p$/) {
                $value .= "";
            }
            default {
                &err_exit("wrong param value format in '$value'");
            }
        }
        my %param = (
                      'arg_name' => $arg_name,
                      'value' => $value,
                    );
        push @params, \%param;
    }
    \@params;
}

sub gen_array {
    my $arr_str = shift;
    my @array = ();
    $arr_str = ~ s/^\[\s+//;
    $arr_str = ~ s/,?\s+\]$//;
    foreach (split ',', $arr_str) {
        s/^\s+//;
        s/\s+$//;
        when (/^$num_p$/) {
            $_ += 0;
        }
        when (/^$bool_p$/) {
            if ($_ eq "true") {
                $_ = \1;
            } else {
                $_ = \0;
            }
        }
        when (/^$str_p$/) {
            $_ =~ s/^"//;
            $_ =~ s/"$//;
        }
        when (/^null$/) {
            $_ = undef;
        }
        default {
            &err_exit("wrong array param element format in '$_'");
        }
        push @array;
    }
    \@array;
}
sub err_exit {
    my $msg = $_[0];
    print STDERR "ERROR: $msg\n";
    exit 1;
}

sub exit_msg {
    my $status = shift;
    my $msg = shift;
    print $msg;
    exit $status;
}

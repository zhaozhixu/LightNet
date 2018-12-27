#! /usr/bin/env perl

use 5.014;
use warnings;
use strict;
use JSON;
use Getopt::Long;
no warnings 'experimental::smartmatch';

my $usage =<<EOF;
Usage: $0 [OPTION...] [INFILE]
Generate JSON-format IR code from INFILE which is in simplified IR format.
Read input from standard input if INFILE is not given.

Options:
  -h, --help      show this help
  -o, --outfile   specify output file name (print to standard out without this)

Author: Zhao Zhixu
EOF

my $outfile = '';
GetOptions(
           'help' => sub {&exit_msg(0, $usage)},
           'outfile=s' => \$outfile,
          ) or &exit_msg(1, $usage);

my $name_p = qr/([a-zA-Z_][a-zA-Z0-9_]*)/;
my $num_p = qr/([-+]?((\d*\.?\d+)|(\d+\.?\d*))([eE][-+]?\d+)?)/;
my $str_p = qr/("(\\.|[^"\\])*")/;
my $bool_p = qr/(true|false)/;
my $type_p = qr/($num_p|$bool_p|$str_p|null)/;
my $arr_p = qr/(\[(\s*$type_p\s*,)*\s*$type_p\s*,?\s*\])/;
my $value_p = qr/($type_p|$arr_p|$name_p)/;
my $kv_p = qr/($name_p\s*:\s*$value_p)/;

my $in_text;
if (@ARGV == 0) {
    $in_text = join '', <STDIN>;
} else {
    my $infile = shift @ARGV;
    open INFILE, '<', $infile or die "Cannot open $infile: $!";
    $in_text = join '', <INFILE>;
    close INFILE;
}

my @ops = ();

push @ops, &gen_op("conv(src:hha,wts:q3|dst: dsf|stride:[2,3],padding:[1,2], hi:\"hi\")");
my %top = (
           'ops' => \@ops,
          );

my $json_obj = JSON->new->pretty();
my $json_str = $json_obj->encode(\%top);
say $json_str;

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

    # TODO: not right when a string contains ','
    my %op = (
              'optype' => $optype,
              'tensors_in' => &gen_tensors($ins_str),
              'tensors_out' => &gen_tensors($outs_str),
              'params' => &gen_params($params_str),
             );
    \%op;
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
    my @array_value = ();
    my $is_array = 0;
    foreach (split /,/, $params_str) {
        if (/^\s*$name_p\s*:\s*\[/) {
            $is_array = 1;
            @array_value = ();
            push @array_value, $_;
            next;
        }
        if ($is_array and not /\]\s*$/) {
            push @array_value, $_;
            next;
        } elsif ($is_array and /\]\s*$/) {
            push @array_value, $_;
            $is_array = 0;
            $_ = join ',', @array_value;
        }
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
    $arr_str =~ s/^\[\s*//;
    $arr_str =~ s/,?\s*\]$//;
    foreach (split ',', $arr_str) {
        s/^\s+//;
        s/\s+$//;
        given ($_) {
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
        }
        push @array, $_;
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

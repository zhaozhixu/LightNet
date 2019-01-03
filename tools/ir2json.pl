#! /usr/bin/env perl

use 5.014;
use warnings;
use strict;
use JSON;
use Getopt::Long;
use File::Copy;
use File::Temp;
no warnings 'experimental::smartmatch';

my $usage =<<EOF;
Usage: $0 [OPTION...] [INFILE]
Generate JSON-format IR code from INFILE which is in simplified IR format.
Read input from standard input if INFILE is not given. It will do the C-style
preprocessing using gcc before the generation.

Options:
  -h, --help       show this help
  -o, --outfile    specify output file name (print to standard out without this)

Author: Zhao Zhixu
EOF

my $outfile = '';
GetOptions(
           'help' => sub {&exit_msg(0, $usage)},
           'outfile=s' => \$outfile,
          ) or &exit_msg(1, $usage);

my $in_text;
if (@ARGV == 0) {
    $in_text = join '', <STDIN>;
} else {
    my $infile = shift @ARGV;
    open INFILE, '<', $infile or die "Cannot open ${infile}: $!";
    $in_text = join '', <INFILE>;
    close INFILE;
}

$in_text = &preprocess($in_text);

my $name_p = qr/([a-zA-Z_][a-zA-Z0-9_]*)/;
my $num_p = qr/([-+]?((\d*\.?\d+)|(\d+\.?\d*))([eE][-+]?\d+)?)/;
my $str_p = qr/("(\\.|[^"\\])*")/;
my $bool_p = qr/(true|false)/;
my $type_p = qr/($num_p|$bool_p|$str_p|null)/;
my $arr_p = qr/(\[(\s*$type_p\s*,)*\s*$type_p\s*,?\s*\])/;
my $value_p = qr/($type_p|$arr_p|$name_p)/;
my $kv_p = qr/($name_p\s*=\s*$value_p)/;

my @states = split /\s*;\s*/, $in_text;
my @ops = ();
foreach (@states) {
    push @ops, &gen_op($_);
}

my %top = (
           'ops' => \@ops,
          );
my $json_obj = JSON->new->pretty()->canonical();
my $json_str = $json_obj->encode(\%top);
if ($outfile) {
    open OUTFILE, '>', $outfile or die "Cannot open $outfile: $!";
    print OUTFILE $json_str;
    close OUTFILE;
} else {
    print $json_str;
}

my %op_num;
sub gen_op {
    my $state = shift;
    my $optype;
    my $name;
    my $ins_str;
    my $outs_str;
    my $params_str;

    my $args_p = qr/((\s*$kv_p\s*,)*\s*$kv_p\s*,?)/;
    my $arg_list_p = qr/((?<ins>$args_p)?\s*\|\s*(?<outs>$args_p)?\s*\|\s*(?<params>$args_p)?)/;
    my $state_p = qr/((?<optype>$name_p)\s*\(\s*$arg_list_p\s*\))/;

    if ($state =~ /^\s*$state_p\s*$/) {
        $optype = $+{optype};
        $op_num{$optype} = exists $op_num{$optype} ? $op_num{$optype}+1 : 0;
        $name = "$optype$op_num{$optype}";
        $ins_str = $+{ins} if exists $+{ins};
        $outs_str = $+{outs} if exists $+{outs};
        $params_str = $+{params} if exists $+{params};
    } else {
        &err_exit("wrong syntax in\n    $state");
    }

    # TODO:FIXME: incorrect when a string contains ','
    my @empty_array = ();
    my %op = (
              'optype' => $optype,
              'name' => $name,
              'tensors_in' => $ins_str ? &gen_tensors($ins_str) : \@empty_array,
              'tensors_out' => $outs_str ? &gen_tensors($outs_str) : \@empty_array,
              'params' => $params_str ? &gen_params($params_str) : \@empty_array,
             );
    \%op;
}

sub gen_tensors {
    my $tensors_str = shift;
    my @tensors = ();
    foreach (split ',', $tensors_str) {
        my ($arg_name, $name) = map {s/^\s+//;s/\s+$//;$_} split '=';
        &err_exit("wrong tensor name format in '$name'")
          unless $name =~ /^$name_p$/;
        my %tensor = (
                      'arg_name' => $arg_name,
                      'name' => $name,
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
        if (/^\s*$name_p\s*=\s*\[/ and not /\]\s*$/) {
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
        my ($arg_name, $value) = map {s/^\s+//;s/\s+$//;$_} split '=';
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

sub preprocess {
    my $in_text = shift;
    my ($tmp_fh, $tmp_name) = mkstemps("XXXXXX", '.c');
    print $tmp_fh $in_text;
    close $tmp_fh;
    my $status = system "gcc -E $tmp_name -o ${tmp_name}.i";
    unlink "${tmp_name}" or die "Cannot unlink file ${tmp_name}: $!";
    &exit_msg(0, "gcc returns non-zero status") if $status;

    open TMP, '<', "${tmp_name}.i" or die "Cannot open ${tmp_name}.i: $!";
    $in_text = join '', <TMP>;
    close TMP;
    unlink "${tmp_name}.i" or die "Cannot unlink file ${tmp_name}.i: $!";
    my @in_lines;
    foreach (split "\n", $in_text) {
        push @in_lines, $_ unless /^#/;
    }
    $in_text = join "\n", @in_lines;
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

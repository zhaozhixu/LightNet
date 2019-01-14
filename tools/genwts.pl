#! /usr/bin/env perl

# The weight file format in this code refers to the weight file format for
# Nvidia TensorRT library.

use warnings;
use strict;
use Getopt::Long;

my $usage = <<EOF;
Usage: $0 -t TYPE -o OUTFILE INFILE(s)

Convert the input text file INFILEs containing weight numbers to one text file
OUTFILE in hexadecimal string format.

A OUTFILE starts with a line of an integer for the number of weight arrays in
this file. Then each weight array is represented in a line, which has the
following fields seperated in spaces: a weight name in C identifier syntax,
an integer for the data type, an integer for the length of the array, and an
array of hexadecimal words of the machine storage format of the weight numbers.

The weight name will be the initial part conforming to C identifier syntax
of the file name of a INFILE. The integer for the data type allows 0 (float),
1 (half), 2 (int8), set with the `type` option (currently it only supports 0
and 2). The integer for the length of the array will be the number of weight
numbers in a INFILE. Weight numbers are literal floating or integer constant
strings seperated by white spaces and some punctuations.

[options]
  -h, --help               print this message
  -t, --type=<type>        data type set for all weights, see above
  -o, --outfile=<outfile>  output file name

Author: Zhao Zhixu
EOF

my $type = '';
my $outfile = '';
GetOptions(
           'help' => sub{&exit_msg(0, $usage)},
           'type=i' => \$type,
           'outfile=s' => \$outfile,
          ) or &exit_msg(1, $usage);

&exit_msg(1, "Need a type option\n$usage") if $type eq '';
&exit_msg(1, "Data type of $type is not supported yet\n$usage")
    if $type != 0 and $type != 2;
&exit_msg(1, "Need a outfile\n$usage") if !$outfile;
my $nweight = @ARGV;
&exit_msg(1, "Need INFILE(s)\n$usage") if $nweight == 0;

open OUTFILE, '>', $outfile or die "Can't open file ${outfile}. ($!)";
print OUTFILE "$nweight\n";

while (my $infile = shift @ARGV) {
    if (not -T $infile) {
        &warn_msg("$infile doesn't look like a \"text\" file");
        next;
    }
    my $wname;
    if ($infile =~ /^(\w+)/) {
        $wname = $1;
    } else {
        &warn_msg("${infile}'s name doesn't start with a prefix conforming to C identifier syntax. Skip it");
        next;
    }
    if (not open INFILE, '<', $infile) {
        &warn_msg("Can't open file ${infile}. ($!) Skip it.");
        next;
    }

    my @numbers;
    my @outputs;
    while (<INFILE>) {
        my @words = split /\s+|[,;!|\[\](){}<>]+/, $_; # TODO
        foreach (@words) {
            push @numbers, $1
                if (/^([-+]?((\d*\.?\d+)|(\d+\.?\d*))([eE][-+]?\d+)?)$/);
        }
    }
    my $nnumbers = @numbers;
    print OUTFILE "$wname $type $nnumbers ";
    my $fmt = "";
    if ($type == 0) {
        $fmt = "f*";
    } elsif ($type == 2) {
        $fmt = "c*";
    } else {
        &exit_msg(1, "unsupported data type $type");
    }
    foreach (@numbers) {
        push @outputs, join "", reverse split /(?=[0-9a-fA-F])/,
            unpack("h*", pack($fmt, $_));
    }
    print OUTFILE (join " ", @outputs) . "\n";
    close INFILE;
}

close OUTFILE;

sub warn_msg {
    my $msg = $_[0];
    print STDERR "WARNING: $msg\n";
}

sub exit_msg {
    my $status = shift;
    my $msg = shift;
    print $msg;
    exit $status;
}

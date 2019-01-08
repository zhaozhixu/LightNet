#! /usr/bin/env perl

use 5.014;
use warnings;
use strict;
use JSON;
use Getopt::Long;
no warnings 'experimental::smartmatch';

my $usage =<<EOF;
Usage: $0 [OPTION...] [INFILE]
Generate JSON-format code from INFILE which is in simplified JSON format.
The simplifications include: keys don't need to be surrounded with double
qoutes("); the last elements in arrays and objects can have a trailing comma;
string values can be heredoc-ed with two triple qoutes ('''), each in a
seperated line, between which special characters can be auto-escaped, such as
newlines, qoutes, etc.
Read input from standard input if INFILE is not given.

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

$in_text = &here_doced($in_text);
say $in_text;

sub here_doced {
    my $intext = shift;
    my @lines = split "\n", $intext;
    my @res = ();
    my $string_line = '';
    foreach (@lines) {
        if ($string_line) {
            if (/'''$/) {
                $string_line .= '"';
                push @res, $string_line;
                $string_line = '';
            } else {
                s/\\/\\\\/g;
                s/(?=[^\\])"/\\"/g;
                s/\t/\\t/g;
                s/\f/\\f/g;
                s/\r/\\r/g;
                $string_line .= "$_\\n";
            }
        } else {
            if (/'''$/) {
                s/'''/"/;
                $string_line = $_;
            } else {
                push @res, $_;
            }
        }
    }
    my $outtext = join "\n", @res;
}

sub keys_surround {
    my $states = shift;
    my @res = ();
    foreach (@$states) {

    }
}

#! /usr/bin/env perl

use 5.014;
use warnings;
use strict;
use Getopt::Long;
use Cwd 'abs_path';
use File::Basename;
use lib abs_path(dirname(__FILE__));
use easyjson 'easyjson::easy_to_json';
no warnings 'experimental::smartmatch';

# NOTE: This is a simple imprecise implementation.
my $usage =<<EOF;
Usage: $0 [OPTION...] [INFILE]
Generate JSON-format code from INFILE which is in easy-to-write JSON format.
Read input from standard input if INFILE is not given.

The extensions of the latter format:
1. The last elements in arrays and objects can have a trailing comma.
2. Comments are supported; every thing from double slashs (//) to the end of
   the line is a comment.
3. String values can be heredoc-ed with two triple qoutes ('''), each in a
   seperated line, between which special characters can be auto-escaped,
   such as newlines, qoutes, etc. The triple qoutes must be at the end of a
   line except that the ending qoutes can have a comma following them.

Options:
  -h, --help       show this help
  -o, --outfile    specify output file name (print to standard out without this)

Author: Zhao Zhixu
EOF

my $outfile = '';
GetOptions(
           'help' => sub{&exit_msg(0, $usage)},
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

my $text = easyjson::easy_to_json($in_text);

if ($outfile) {
    open OUTFILE, '>', $outfile or die "Cannot open $outfile: $!";
    print OUTFILE $text;
    close OUTFILE;
} else {
    print $text;
}

sub exit_msg {
    my $status = shift;
    my $msg = shift;
    print $msg;
    exit $status;
}

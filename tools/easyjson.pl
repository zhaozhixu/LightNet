#! /usr/bin/env perl

use 5.014;
use warnings;
use strict;
use JSON;
use Getopt::Long;
no warnings 'experimental::smartmatch';

# NOTE: This is a simple imprecise implementation.
my $usage =<<EOF;
Usage: $0 [OPTION...] [INFILE]
Generate JSON-format code from INFILE which is in easy-to-write JSON format.
Read input from standard input if INFILE is not given.

The differences between two formats:
1. The last elements in arrays and objects can have a trailing comma.
2. Comments are supported; every thing from double slashs (//) to the end of
   the line is a comment.
3. String values can be heredoc-ed with two triple qoutes ('''), each in a
   seperated line, between which special characters can be auto-escaped,
   such as newlines, qoutes, etc. The qoutes must be at the end of a line
   except that the ending qoutes can have a comma following them.

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

my $text = &do_heredoc($in_text);
$text = &do_comma_comment($text);

if ($outfile) {
    open OUTFILE, '>', $outfile or die "Cannot open $outfile: $!";
    print OUTFILE $text;
    close OUTFILE;
} else {
    print $text;
}

# TODO: triple quotes cannot exist in a heredoc yet
sub do_heredoc {
    my $in = shift;
    my @lines = split "\n", $in;
    my @res = ();
    my @string_lines = ();
    my $string_line = '';
    foreach (@lines) {
        if ($string_line) {
            if (/'''(?<comma>\s*,\s*)?$/) {
                $string_line .= join "\\n", @string_lines;
                $string_line .= "\"";
                $string_line .= $+{comma} if exists $+{comma};
                push @res, $string_line;
                @string_lines = ();
                $string_line = '';
            } else {
                s/\\/\\\\/g;
                s/(?=[^\\])"/\\"/g;
                s/\t/\\t/g;
                s/\f/\\f/g;
                s/\r/\\r/g;
                push @string_lines, $_;
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
    &err_exit("unmatched triple quotes (''')") if $string_line;
    my $out = join "\n", @res;
}

sub do_comma_comment {
    my $in = shift;
    $in =~ s/,(\s*[\]}])/$1/g;     # TODO: not handle commas in strings yet
    my @lines = split "\n", $in;
    my @res = ();
    foreach (@lines) {
        s/\/\/.*$//;
        push @res, $_;
    }
    my $out = join "\n", @res;
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

#! /usr/bin/env perl

package easyjson;

use 5.014;
use warnings;
use strict;
use Getopt::Long;
no warnings 'experimental::smartmatch';

sub easy_to_json {
    my $text = shift;
    $text = &do_heredoc($text);
    $text = &do_comma_comment($text);
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
    $in =~ s/,(\s*[\]}])/$1/g;     # TODO: no handle commas in strings yet
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

1;

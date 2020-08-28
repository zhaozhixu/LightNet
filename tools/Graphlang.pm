#! /usr/bin/env perl

package Graphlang;

use 5.014;
use warnings;
use strict;
use Parse::RecDescent;
use Data::Dumper;

$::RD_ERRORS = 1; # Make sure the parser dies when it encounters an error
$::RD_WARN   = 3; # Enable warnings. This will warn on unused rules &c.
$::RD_HINT   = 1;
# $::RD_TRACE  = 1;

my @types = ("number", "string", "number\\[\\d\\]", "string\\[\\d\\]");
my %types = map { $_ => 1 } @types;

my $grammar = <<'_EOGRAMMAR_';

{ use 5.014; }

identifier : /[A-Za-z_]\w*/

number : /[-+]?((\d*\.\d+)|(\d+\.?\d*))([eE][-+]?\d+)?/
    { $return = { type => ""}; }

double_quoted_chars : /(\\.|[^"\\])+/s

string : '"' '"'
    { $return = "" }
    | '"' (double_quoted_chars | ...!double_quoted_chars) ('"' | ...!'"')
    {
        if ($item[2]) {
            if ($item[3]) {
                $return = Graphlang::escape($item[2]);
            } else {
                Graphlang::syntax_error($item[0], $thisline, $thiscolumn, $thisoffset, "'\"'");
                $return = undef;
            }
        } else {
            Graphlang::syntax_error($item[0], $thisline, $thiscolumn, $thisoffset,
                                    'double quoted chars', "'\"'");
            $return = undef;
        }
    }

primary_expression : identifier | number | string
    | '(' conditional_expression ')'
    { $return = "($item[2])"; }

right_accessor : identifier '.' identifier '[' identifier ']'
    { $return = Graphlang::expand_right_accessor(); }

identifier_with_index : /[A-Za-z_]\w*(\$[\@\^])?\w*/

left_accessor : identifier '.' identifier '[' identifier_with_index ']'

edge : right_accessor '=>' right_accessor
    { $return = join ' ', @item[1..$#item]; }

directive_call : '${' identifier conditional_expression(s /,/) '}'

postfix_expression : primary_expression | right_accessor | left_accessor | edge | directive_call

not_expression : '!' postfix_expression
    { $return = "!$item[2]"; }
    | postfix_expression

multiplicative_expression : not_expression * multiplicative_expression
    { $return = "$item[1] * $item[2]"; }
    | not_expression / multiplicative_expression
    { $return = "$item[1] / $item[2]"; }
    | not_expression % multiplicative_expression
    { $return = "$item[1] % $item[2]"; }
    | not_expression

additive_expression : multiplicative_expression + additive_expression
    { $return = "$item[1] + $item[2]"; }
    | multiplicative_expression - additive_expression
    { $return = "$item[1] - $item[2]"; }
    | multiplicative_expression

relational_expression : additive_expression < relational_expression
    | additive_expression <= relational_expression
    | additive_expression > relational_expression
    | additive_expression >= relational_expression
    | additive_expression

equality_expression : relational_expression == equality_expression
    | relational_expression != equality_expression
    | relational_expression

and_expression : equality_expression '&&' and_expression
    | equality_expression

or_expression : and_expression '||' or_expression
    | or_expression

conditional_expression : or_expression '?' conditional_expression : conditional_expression
    | or_expression

assignment_expression : left_accessor '=' right_accessor

_EOGRAMMAR_

our $code;
our $file;
our @errors;

$code = join "", <>;
$Data::Dumper::Indent = 1;
say Dumper(parse($code));
# say Dumper($desc_table);
# print STDERR join "", @errors;

sub parse {
    my $text = shift;
    $file = @_ == 1 ? shift : "";

    my $parser = Parse::RecDescent->new($grammar) or die "Bad grammer!\n";
    $code = $text;
    if (defined $parser->condition($text)) {
        return $parser->condition($text);
    } else {
        return undef;
    }
}

sub error {
    my ($rule_name, $line, $column, $offset, $err_str) = @_;

    $rule_name =~ s/_/ /g;
    my $text = $code;
    my $find = "\n";
    if (substr($text, $offset) =~ /(.{0,70})\n/) {
        $find = "\n    $1";
        $find = "$find ..." if length($1) == 70;
        $find .= "\n";
    }

    my $msg = "$line:$column: Invalid $rule_name: $err_str$find";
    push @errors, $msg;
}

sub syntax_error {
    my ($rule_name, $line, $column, $offset, @expects) = @_;

    my $expects_str = "";
    $expects_str = "expecting ".(join " or ", @expects) if @expects > 0;

    error($rule_name, $line, $column, $offset, $expects_str);
}

sub escape {
    my $str = shift;

    my %tr = (
              n => "\n",
              r => "\r",
              t => "\t",
              f => "\f",
              b => "\b",
             );

    $str =~ s{\\(?:(\W)|(.))}{
        defined($1) ? $1:
        defined($tr{$2}) ? $tr{$2} : $2
    }seg;

    $str;
}

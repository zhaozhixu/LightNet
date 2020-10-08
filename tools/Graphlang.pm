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
$::RD_TRACE  = 1;

my %global_ops;
my @types = ("number", "string", "number\\[\\d\\]", "string\\[\\d\\]");
my %types = map { $_ => 1 } @types;

my $grammar = <<'_EOGRAMMAR_';

{ use 5.014; }

identifier : /[A-Za-z_]\w*/

number : /[-+]?((\d*\.\d+)|(\d+\.?\d*))([eE][-+]?\d+)?/

string : '"' <commit> double_quoted_chars <commit> '"'
    { $return = Graphlang::escape($item[3]); }
    | "'" <commit> single_quoted_chars <commit> "'"
    { $return = Graphlang::escape_char($item[3], "'"); }
    | '`' <commit> back_quoted_chars <commit> '`'
    { $return = Graphlang::escape_char($item[3], '`'); }
    | '~' <commit> tilde_quoted_chars <commit> '~'
    { $return = Graphlang::escape_char($item[3], '~'); }
    | <error?><reject>

double_quoted_chars : /(\\.|[^"\\])*/s

single_quoted_chars : /(\\'|[^'])*/s

back_quoted_chars : /(\\`|[^`])*/s

tilde_quoted_chars : /(\\~|[^~])*/s

target : postfix_expression /\s*\Z/
    { $return = $item[1]; }
    | <error>

primary_expression : identifier | number | string | parenthese_expression

right_accessor : identifier '.' identifier '[' identifier ']'

identifier_with_index : /[A-Za-z_]\w*(\$[\@\^])?\w*/

left_accessor : identifier '.' identifier '[' identifier_with_index ']'

edge : right_accessor '=>' right_accessor
    { $return = join ' ', @item[1..$#item]; }

directive_call : '${' identifier primary_expression(s /,/) '}'

postfix_expression : primary_expression | right_accessor | left_accessor | edge | directive_call

not_expression : '!' postfix_expression
    { $return = "!$item[2]"; }
    | postfix_expression
    | <error>

multiplicative_expression : <leftop: not_expression /([*\/%])/  not_expression>
    { $return = join "", @{$item[1]}; }
    | <error>

additive_expression : <leftop: multiplicative_expression /([+-])/  multiplicative_expression>
    { $return = join "", @{$item[1]}; }
    | <error>

relational_expression : <leftop: additive_expression /(<|<=|>|>=)/  additive_expression>
    { $return = join "", @{$item[1]}; }
    | <error>

equality_expression : <leftop: relational_expression /(==|!=)/  relational_expression>
    { $return = join "", @{$item[1]}; }
    | <error>

and_expression : <leftop: equality_expression /(&&)/ equality_expression>
    { $return = join "", @{$item[1]}; }
    | <error>

or_expression : <leftop: and_expression /(\|\|)/ and_expression>
    { $return = join "", @{$item[1]}; }
    | <error>

conditional_expression : or_expression '?' conditional_expression ':' conditional_expression
    | or_expression

parenthese_expression : '(' <commit> conditional_expression <commit> ')'
    { $return = "($item[3])"; }
    | <error?><reject>

assignment_expression : left_accessor '=' right_accessor

_EOGRAMMAR_

our $code;
our $file;
our @errors;

$code = join "", <>;
$Data::Dumper::Indent = 1;
say Dumper(parse($code));
# say Dumper($desc_table);
print STDERR join "", @errors;

sub parse {
    my $text = shift;
    $file = @_ == 1 ? shift : "";

    my $parser = Parse::RecDescent->new($grammar) or die "Bad grammer!\n";
    $code = $text;
    if (defined $parser->target($text)) {
        return $parser->target($text);
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

sub escape_char {
    my $str = shift;
    my $char = shift;

    $str =~ s/\\(?:($char))/$1/seg;

    $str;
}

sub find_op_desc {
    my $optype = shift;
    my $op = $global_ops{$optype};
    if (not $op) {
        my $opdir = abs_path(dirname(__FILE__))."/../protos/op";
        my @possible_files = possible_op_files($optype);
        foreach (@possible_files) {
            my $file = "$opdir/$_";
            next unless -e $file;
            read_op_desc($file, \%global_ops);
            if (exists $global_ops{$optype}) {
                $op = $global_ops{$optype};
                last;
            }
        }
        if (not $op) {
            util::err_exit("Cannot find the description JSON for optype '$optype'");
        }
    }

    return $op;
}

sub possible_op_files {
    my $optype = shift;

    my @names = ();
    push @names, $optype.'.op';
    my @words = split '_', $optype;
    push @names, (join '_', @words[0..$#words-1]).'.op';

    return @names;
}

sub read_op_desc {
    my $file = shift;
    my $hash = shift;

    open OPDESC_FILE, '<', $file or die "Cannot open $file: $!";
    my $opdesc_text = join '', <OPDESC_FILE>;
    close OPDESC_FILE;
    my $opdesc = Opdesc::parse($opdesc_text, $file);
    if (not defined $opdesc) {
        die (join "", map { "ERROR: $file: $_" } @Opdesc::errors);
    }
    foreach (keys %$opdesc) {
        $hash->{$_} = $opdesc->{$_};
    }
}

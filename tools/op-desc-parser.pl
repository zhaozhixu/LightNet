#! /usr/bin/env perl

use 5.014;
use strict;
use warnings;
use Parse::RecDescent;

$::RD_ERRORS = 1; # Make sure the parser dies when it encounters an error
$::RD_WARN   = 1; # Enable warnings. This will warn on unused rules &c.
$::RD_HINT   = 1; # Give out hints to help fix problems.

our %vars;

my $grammar = <<'_EOGRAMMAR_';

INTEGER : /[-+]?\d+/
FLOAT : /[-+]?((\d*\.\d+)|(\d+\.?\d*))([eE][-+]?\d+)?/
STRING : /"(\\.|[^"\\])*"/
BOOL : /true|false/
IDENTIFIER : /[A-Za-z_]+\w*/

primary_expression : INTEGER
    | FLOAT
    | STRING
    | BOOL
    | IDENTIFIER
    | '(' expression ')'

postfix_expression : primary_expression
    | postfix_expression '[' expression ']'
    | postfix_expression '(' argument_expression_list ')'
    | postfix_expression '.' IDENTIFIER
    | postfix_expression '=>' IDENTIFIER

argument_expression_list : <leftop: assignment_expression ',' assignment_expression>

unary_expression : postfix_expression
    | '-' unary_expression
    | '+' unary_expression
    | '!' unary_expression

multiplicative_expression : <leftop: unary_expression '*' unary_expression>
    | <leftop: unary_expression '/' unary_expression>
    | <leftop: unary_expression '%' unary_expression>

additive_expression : <leftop: multiplicative_expression '+' multiplicative_expression>
    | <leftop: multiplicative_expression '-' multiplicative_expression>

relational_expression : <leftop: additive_expression '<' additive_expression>
    | <leftop: additive_expression '<=' additive_expression>
    | <leftop: additive_expression '>' additive_expression>
    | <leftop: additive_expression '>=' additive_expression>

equality_expression : <leftop: relational_expression '==' relational_expression>
    | <leftop: relational_expression '!=' relational_expression>

AND_expression : <leftop: equality_expression '&&' equality_expression>

OR_expression : <leftop: AND_expression '||' AND_expression>

assignment_expression : <rightop: assignment_expression '=' unary_expression>

expression : <leftop: assignment_expression ',' assignment_expression>

declaration : declaration_specifier init_declarator_list

declaration_specifier : type_specifier declaration_specifier
    | functional_specifier declaration_specifier

init_declarator_list : init_declarator
    | init_declarator_list init_declarator

init_declarator : declarator
    | declarator '=' initializer

type_specifier : 'Graph'
    | 'List'
    | 'Operator'
    | 'int'
    | 'float'
    | 'bool'
    | 'string'

_EOGRAMMAR_

my $parser = Parse::RecDescent->new($grammar);

while (<>) {
    $parser->startrule($_);
}

sub eval_expression {
    print "sub: ".(join " ", @_)."\n";
    shift;
    my ($lhs, $op, $rhs) = @_;
    $lhs = $vars{$lhs} if $lhs=~/[^-+0-9]/;
    eval "$lhs $op $rhs";
}

print "a=2\n";             $parser->startrule("a=2");
print "a=1+3\n";           $parser->startrule("a=1+3");
print "print 5*7\n";       $parser->startrule("print 5*7");
print "print 2/4\n";       $parser->startrule("print 2/4");
print "print 2+2/4\n";     $parser->startrule("print 2+2/4");
print "print 2+-2/4\n";    $parser->startrule("print 2+-2/4");
print "a = 5 ; print a\n"; $parser->startrule("a = 5 ; print a");

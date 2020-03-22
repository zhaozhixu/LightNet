#! /usr/bin/env perl

use 5.014;
use strict;
use warnings;
use Parse::RecDescent;
use Data::Dumper;
use JSON -convert_blessed_universally;;

$::RD_ERRORS = 1; # Make sure the parser dies when it encounters an error
$::RD_WARN   = 1; # Enable warnings. This will warn on unused rules &c.
# $::RD_HINT   = 1;
# $::RD_TRACE  = 1;

our $parse_tree;
our $desc_table;

my $grammar = <<'_EOGRAMMAR_';

{ use 5.014; }

number : /[-+]?((\d*\.\d+)|(\d+\.?\d*))([eE][-+]?\d+)?/

string : '"' /(\\.|[^"\\])*/s '"'
    { $return = &main::escape($item[2]) }
    | '"' <commit>
    { &main::error($item[0], $thisline, $thiscolumn, $thisoffset, "'\"'") } <reject>
    | code_block
    | '`' /([^`])*/  '`'
    { $return = $item[2] }

bool : /true|false/
    { $return = $item[1] eq 'true' ? 1 : 0}

identifier : /[A-Za-z_]+\w*/

null : 'null'
    { $return = undef; 1}

delete : '-'

code_start : /```\n/

code_end : /```(?=\s*?,?[ \t\f\r]*\n)/

code : /.*?(?=\n\s*?```\s*?,?[ \t\f\r]*\n)/s

code_block : code_start code code_end
    { $return = $item{code} }

operator_description : element
    { $main::desc_table = $item{element}; }

value : object
    { $return = {type => "object", value => $item[1]} }
    | array
    { $return = {type => "array", value => $item[1]} }
    | string
    { $return = {type => "string", value => $item[1]} }
    | number
    { $return = {type => "number", value => $item[1]} }
    | bool
    { $return = {type => "bool", value => $item[1]} }
    | null
    { $return = {type => "null", value => $item[1]} }
    | delete
    { $return = {type => "delete", value => $item[1]} }

object : '{' '}'
    { $return = {} }
    |'{' (members | ...!members) /,?/ ('}' | ...!'}')
    {
        if ($item[2]) {
            if ($item[4]) {
                my %object_hash;
                map { $object_hash{$_->{key}} = $_->{value} } @{$item[2]};
                $return = \%object_hash;
            } else {
                &main::error($item[0], $thisline, $thiscolumn, $thisoffset, "','", "'}'");
                $return = undef;
            }
        } else {
            &main::error($item[0], $thisline, $thiscolumn, $thisoffset, 'members', "'}'");
            $return = undef;
        }
    }

members : member(s /,/)
    { $return = $item[1] }

member : identifier ':' element
    { $return = {key => $item{identifier}, value => $item{element}} }
    | identifier ...!':'
    { &main::error($item[0], $thisline, $thiscolumn, $thisoffset, "':'") } <reject>

array : '[' ']'
    { $return = [] }
    |'[' (elements | ...!elements) /,?/ (']' | ...!']')
    {
        if ($item[2]) {
            if ($item[4]) {
                $return = $item[2];
            } else {
                &main::error($item[0], $thisline, $thiscolumn, $thisoffset, "','", "']'");
                $return = undef;
            }
        } else {
            &main::error($item[0], $thisline, $thiscolumn, $thisoffset, 'elements', "']'");
            $return = undef;
        }
    }

elements : element(s /,/)
    { $return = $item[1] }

element : value
    {
        $return = {line => $thisline, column => $thiscolumn,
                   type => $item{value}->{type}, value => $item{value}->{value}}
    }

_EOGRAMMAR_

my $parser = Parse::RecDescent->new($grammar) or die "Bad grammer!";

my $code = join "", <>;

defined $parser->operator_description($code) or say "Bad source text!";
$Data::Dumper::Indent = 1;
say Dumper($desc_table);
# say $desc_table->{pre_run};
# say $desc_table->{value}->{post_run}->{value};

sub error {
    my $rule_name = shift;
    my $line = shift;
    my $column = shift;
    my $offset = shift;
    my @expects = @_;

    $rule_name =~ s/_/ /g;
    my $text = $code;
    my $find = "";
    if (substr($text, $offset) =~ /(.{0,70})\n/) {
        $find = "\n    $1";
        $find = "$find ..." if length($1) == 70;
        $find .= "\n";
    }
    my $expects_str = "";
    $expects_str = ", expecting ".(join " or ", @expects) if @expects > 0;

    my $msg = "Error at $line:$column: Invalid $rule_name$expects_str$find";
    print STDERR $msg;
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

#! /usr/bin/env perl

package Opdesc;

use 5.014;
use strict;
use warnings;
use Parse::RecDescent;
use Data::Dumper;
use constant HASH => ref {};
use constant ARRAY => ref [];

$::RD_ERRORS = 1; # Make sure the parser dies when it encounters an error
$::RD_WARN   = 3; # Enable warnings. This will warn on unused rules &c.
$::RD_HINT   = 1;
# $::RD_TRACE  = 1;

my $grammar = <<'_EOGRAMMAR_';

{ use 5.014; }

number : /[-+]?((\d*\.\d+)|(\d+\.?\d*))([eE][-+]?\d+)?/

string : '"' '"'
    { $return = "" }
    | '"' (double_quoted_chars | ...!double_quoted_chars) ('"' | ...!'"')
    {
        if ($item[2]) {
            if ($item[3]) {
                $return = Opdesc::escape($item[2]);
            } else {
                Opdesc::syntax_error($item[0], $thisline, $thiscolumn, $thisoffset, "'\"'");
                $return = undef;
            }
        } else {
            Opdesc::syntax_error($item[0], $thisline, $thiscolumn, $thisoffset,
                                 'double quoted chars', "'\"'");
            $return = undef;
        }
    }
    | "'" "'"
    { $return = "" }
    | "'" (single_quoted_chars | ...!single_quoted_chars) ("'" | ...!"'")
    {
        if ($item[2]) {
            if ($item[3]) {
                $return = $item[2];
            } else {
                Opdesc::syntax_error($item[0], $thisline, $thiscolumn, $thisoffset, "''''");
                $return = undef;
            }
        } else {
            Opdesc::syntax_error($item[0], $thisline, $thiscolumn, $thisoffset,
                                 'single quoted chars', "'''");
            $return = undef;
        }
    }
    | '`' '`'
    { $return = "" }
    | '`' (back_quoted_chars | ...!back_quoted_chars) ('`' | ...!'`')
    {
        if ($item[2]) {
            if ($item[3]) {
                $return = $item[2];
            } else {
                Opdesc::syntax_error($item[0], $thisline, $thiscolumn, $thisoffset, "'`'");
                $return = undef;
            }
        } else {
            Opdesc::syntax_error($item[0], $thisline, $thiscolumn, $thisoffset,
                                 'back quoted chars', "'`'");
            $return = undef;
        }
    }

double_quoted_chars : /(\\.|[^"\\])+/s

single_quoted_chars : /(\\'|[^'])+/s

back_quoted_chars : /(\\`|[^`])+/s

bool : /true|false/
    { $return = $item[1] eq 'true' ? 1 : 0}

identifier : /[A-Za-z_]+\w*/

null : 'null'
    { $return = 0 }

delete : '-'

operator_description : object /^\Z/
    { $Opdesc::desc_table = $item{object}; }

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
                my $object_hash;
                foreach (@{$item[2]}) {
                      if (exists $object_hash->{$_->{key}}) {
                          Opdesc::error($item[0], $thisline, $thiscolumn, $thisoffset,
                                        "duplicated identifier '$_->{key}'");
                          undef $object_hash;
                          last;
                      }
                      $object_hash->{$_->{key}} = $_->{value}->{value};
                      Opdesc::add_info(\$object_hash->{$_->{key}}, $_->{value});
                 };
                $return = $object_hash;
            } else {
                Opdesc::syntax_error($item[0], $thisline, $thiscolumn, $thisoffset,
                                     "','", "'}'");
                $return = undef;
            }
        } else {
            Opdesc::syntax_error($item[0], $thisline, $thiscolumn, $thisoffset,
                                 'members', "'}'");
            $return = undef;
        }
    }

members : member(s /,/)
    { $return = $item[1] }

member : identifier ':' element
    { $return = {key => $item{identifier}, value => $item{element}} }
    | identifier (':' | ...!':')
    {
        if ($item[2]) {
            Opdesc::syntax_error($item[0], $thisline, $thiscolumn, $thisoffset, "element");
            $return = undef;
        } else {
            Opdesc::syntax_error($item[0], $thisline, $thiscolumn, $thisoffset, "':'");
            $return = undef;
        }
    }

array : '[' ']'
    { $return = [] }
    |'[' (elements | ...!elements) /,?/ (']' | ...!']')
    {
        if ($item[2]) {
            if ($item[4]) {
                my @array;
                for (my $i = 0; $i < @{$item[2]}; $i++) {
                    $array[$i] = $item[2]->[$i]->{value};
                    Opdesc::add_info(\$array[$i], $item[2]->[$i]);
                }
                $return = \@array;
            } else {
                Opdesc::syntax_error($item[0], $thisline, $thiscolumn, $thisoffset,
                                     "','", "']'");
                $return = undef;
            }
        } else {
            Opdesc::syntax_error($item[0], $thisline, $thiscolumn, $thisoffset,
                                 'elements', "']'");
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

our $desc_table;
our $desc_info_table;
our $code;
our @errors;

# $code = join "", <>;
# $Data::Dumper::Indent = 1;
# &parse($code);
# say Dumper($desc_table);
# say ($desc_table->{ops}->[0]->{check});
# say Dumper($desc_info_table);
# say Dumper($desc_info_table->{\$desc_table->{ops}->[1]->{params}});
# print STDERR join "", @errors;

sub parse {
    my $text = shift;

    my $parser = Parse::RecDescent->new($grammar) or die "Bad grammer!\n";
    $code = $text;
    $text = &remove_comment($text);
    if (defined $parser->operator_description($text)) {
        return $desc_table;
    } else {
        return undef;
    }
}

sub add_info {
    my $value_ref = shift;
    my $info = shift;

    $desc_info_table->{$value_ref} = { type => $info->{type},
                                       line => $info->{line},
                                       column => $info->{column} };
}

sub error {
    my $rule_name = shift;
    my $line = shift;
    my $column = shift;
    my $offset = shift;
    my $err_str = shift;

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

    &error($rule_name, $line, $column, $offset, $expects_str);
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

sub remove_comment {
    my $data = shift;

    $data =~ s{   # First, we'll list things we want
                  # to match, but not throw away
                  (
                  #     (?:/[^\r\n\*\/]+/)                     # Match RegExp
                  # |                                          # -or-
                      [^"`/]+                                # other stuff
                  |                                          # -or-
                      (?:"[^"\\]*(?:\\.[^"\\]*)*" [^"`/]*)+  # double quoted string
                  |                                          # -or-
                      (?:`[^`\\]*(?:\\.[^`\\]*)*` [^"`/]*)+  # back quoted constant
                          )

              |

                  # or we'll match a comment. Since it's not in the
                  # $1 parentheses above, the comments will disappear
                  # when we use $1 as the replacement text.

                  /                      # (all comments start with a slash)
                  (?:
                  #     \*[^*]*\*+(?:[^/*][^*]*\*+)*/ # traditional C comments
                  # |                                 # -or-
                      /[^\n]*                       # C++ //-style comments
                  )

          }{ defined $1 ? $1 : "" }gsex;
    $data;
}

1;

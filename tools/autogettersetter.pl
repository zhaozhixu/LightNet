#! /usr/bin/env perl

use 5.014;
use warnings;
use strict;
use Getopt::Long;

my $usage = <<EOF;
Usage: $0 OPTION DIR MODULE STRUCT
Automatically generate getters and/or setters of structure STRUCT in module
MOUDLE. Add function declarations to DIR/MODULE.h and implementaions to
DIR/MODULE.c. Which to be genereated is depended on OPTION.
Declarations are put before /* end of getters/setters declarations */.
Implementaions are put before /* end of getters/setters implementaions */.

Options:
  -h, --help              print this message
  -s, --setter            generate setters
  -g, --getter            generate getters
Author: Zhao Zhixu
EOF

GetOptions(
           'header'
          )

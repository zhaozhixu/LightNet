#! /usr/bin/perl

use warnings;
use strict;
use File::Copy;
use Cwd 'abs_path';

my $usage = <<EOF;
Usage: $0 ROOT OP_NAME
Generate code templates for a new op.
ROOT is the path of the project root.
OP_NAME is the name of the new op.

Example:
	scripts/addtest.pl . slice

	Executing this example from project root will generate code templates
	in file ROOT/src/ln_op_slice.c, and add associated init ops in
	ROOT/src/ln_oplist.c.
EOF
if (@ARGV < 2) {
  print $usage;
  exit;
}
my $root = abs_path($ARGV[0]);
my $op_name = $ARGV[1];

my $op_tpl = <<EOF;
#include <assert.h>
#include "ln_op.h"

/*
 * This function should do the parameter checking and tensor memory allocation.
 */
static void ${op_name}_pre_run(ln_op_arg *op_arg, ln_error **error)
{

     /* check tensors and parameters */
     /* ...... */

     /* allocate tensor memory in need */
     /* ...... */
}

/*
 * Normally we should only do the calculations here. Operations with memory
 * and such should go in pre_run().
 */
static void ${op_name}_run(ln_op_arg *op_arg, ln_error **error)
{

     /* Get tensors and parameters, which should have been checked in pre_run().
        Further errors should be considered as bugs, so we use asserts to catch
        return value. */
     /* ...... */

     /* do the real work */
     /* ...... */
}

/*
 * This function should free all tensor memory pre_run() and run() allocated.
 */
static void ${op_name}_post_run(ln_op_arg *op_arg, ln_error **error)
{

     /* free the tensor memory allocated in pre_run() and run() */
     /* ..... */
}

static ln_op_arg op_arg_${op_name} = {
     .name = NULL,
     .optype = "${op_name}",
     .tensors = NULL,
     .params = NULL,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_op_slice = {
     .op_arg = &op_arg_${op_name},
     .pre_run = ${op_name}_pre_run,
     .run = ${op_name}_run,
     .post_run = ${op_name}_post_run
};
EOF

my $op_file = "$root/src/ln_op_${op_name}.c";
if (-e $op_file) {
  copy($op_file, "$op_file.bak")
    or die "Cannot backup file $op_file: $!";
}
open OP, '>', $op_file
  or die "Cannot open $op_file: $!";
print OP $op_tpl;
close OP;

my $declare = "extern ln_op ln_op_${op_name};";
my $item = "&ln_op_${op_name},";
my $oplist_file = "$root/src/ln_oplist.c";
copy($oplist_file, "$oplist_file.bak")
  or die "Cannot backup file $oplist_file: $!";
open OPLIST_BAK, '<', "$oplist_file.bak"
  or die "Cannot open $oplist_file.bak: $!";
open OPLIST, '>', $oplist_file
  or die "Cannot open $oplist_file: $!";
while (<OPLIST_BAK>) {
  s|/\* end of extern declarations \*/|$declare\n/* end of declarations */|;
  s|/\* end of init ops \*/|$item\n     /* end of init ops */|;
  print OPLIST;
}
close OPLIST;
close OPLIST_BAK;

#! /usr/bin/perl

use warnings;
use strict;
use JSON;
use File::Copy;
use Cwd 'abs_path';

my $usage = <<EOF;
Usage: $0 ROOT OP_JSON_FILE;
ROOT is the path of the project root.
OP_JSON_FILE is the location of op defination file.

Example:
	tools/addop_extend.pl . slice.json

	Executing this from project root will generate code templates
	in file ROOT/src/ln_opimpl_slice_cuda.c from slice.json, and add
	associated init ops in ROOT/src/ln_arch_cpu.c.
EOF
if (@ARGV < 2) {
  print $usage;
  exit;
}
my $root = abs_path($ARGV[0]);
my $json_file = $ARGV[1];
open JSON_FILE, '<', $json_file or die "Cannot open $json_file: $!";
my $json_text = join '', <JSON_FILE>;
close JSON_FILE;

my $json = decode_json $json_text;
&gen_op($json->{ops}[0]);

sub gen_op {
  my $op = $_[0];
  my $optype = $op->{optype};
  my $subfix = "";
  $subfix = $op->{subfix} if exists $op->{subfix};
  my $tensors_in = $op->{tensors_in};
  my $tensors_out = $op->{tensors_out};
  my $params = $op->{params};

  if ($subfix ne "" and not $optype =~ /\w+_$subfix/) {
    &err_exit("optype \"$optype\" doesn't match subfix \"$subfix\"");
  }
  if ($subfix ne "" && $subfix ne "cuda") {
    &err_exit("unsupported subfix \"$subfix\"");
  }
  if ($subfix eq "") {
    $subfix = "cpu";
  }

  my $struct_def = &gen_struct_def($op);print$struct_def;
  my $pre_run_local_vars = "";
  my $pre_run_checks = "";
  my $output_tensor_def = "";
  my $priv_assigns = "";
  my $post_run_code = "";

  my $op_tpl = <<EOF;
/*
 * Copyright (c) 2018 Zhao Zhixu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <assert.h>
#include "ln_op.h"

${struct_def}

/*
 * This function should do the parameter checking and tensor shape inference.
 */
static void ${optype}_pre_run(ln_op_arg *op_arg, ln_error **error)
{
${pre_run_local_vars}

     /* check tensors and parameters */
${pre_run_checks}

     /* define output tensor shape, tensor data should be NULL */
${output_tensor_def}

     /* use op_arg->priv to store private data to be used in other functions */
${priv_assigns}
}

/*
 * This function should only do the calculations.
 */
static void ${optype}_run(ln_op_arg *op_arg, ln_error **error)
{
     /* TODO: add ${optype}_run */
}

/*
 * This function should free all the memory allocated by other *_run()s.
 */
static void ${optype}_post_run(ln_op_arg *op_arg, ln_error **error)
{
${post_run_code}
}

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_${optype} = {
     .optype = "${optype}",
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_${optype} = {
     .op_arg = &op_arg_${optype},
     .pre_run = ${optype}_pre_run,
     .static_run = NULL,
     .run = ${optype}_run,
     .post_run = ${optype}_post_run
};
EOF
}

sub gen_struct_def {
  my $op = $_[0];
  my $tensors_in = $op->{tensors_in};
  my $tensors_out = $op->{tensors_out};
  my $params = $op->{params};

  my @defs = ();
  foreach (@$tensors_in) {
    push @defs, "tl_tensor *$_->{arg_name};";
  }
  foreach (@$tensors_out) {
    push @defs, "tl_tensor *$_->{arg_name};";
    push @defs, "char *$_->{arg_name}_name;";
  }
  foreach (@$params) {
    my $real_type;
    if ($_->{ptype} eq "LN_PARAM_NULL") {
      $real_type = "void *";
    } elsif ($_->{ptype} eq "LN_PARAM_STRING") {
      $real_type = "char *";
    } elsif ($_->{ptype} eq "LN_PARAM_NUMBER") {
      if ($_->{real_type} eq "float") {
        $real_type = "float ";
      } elsif ($_->{real_type} eq "double") {
        $real_type = "double ";
      } elsif ($_->{real_type} eq "int") {
        $real_type = "int ";
      }
    } elsif ($_->{ptype} eq "LN_PARAM_BOOL") {
      $real_type = "tl_bool_t ";
    } elsif ($_->{ptype} eq "LN_PARAM_ARRAY_STRING") {
      $real_type = "char **";
    } elsif ($_->{ptype} eq "LN_PARAM_ARRAY_NUMBER") {
      if ($_->{real_type} eq "float") {
        $real_type = "float *";
      } elsif ($_->{real_type} eq "double") {
        $real_type = "double *";
      } elsif ($_->{real_type} eq "int") {
        $real_type = "int *";
      }
    } elsif ($_->{ptype} eq "LN_PARAM_ARRAY_BOOL") {
      $real_type = "tl_bool_t *";
    } else {
      &err_exit("invalid 'ptype'");
    }
    push @defs, "${real_type}$_->{arg_name};";
  }
  &make_defs_neat(5, \@defs);

  my $defs_str = join "\n", @defs;
  my $struct_def_tpl = <<EOF;
struct priv_s {
${defs_str}
};
EOF
}

sub make_defs_neat {
  my $nspaces = shift;
  my $defs = shift;
  my $max_offset = 0;
  foreach (@$defs) {
    if (/( |\*)(\w+;)/) {
      my $offset = index($_, $2);
      $max_offset = $max_offset < $offset ? $offset : $max_offset;
    } else {
      &err_exit("wrong defs format");
    }
  }
  foreach (@$defs) {
    my $type = $1 if /(\w+)/;
    my $nstars = 0;
    $nstars = length $1 if /(\*+)/;
    my $rest = $2 if /( |\*)(\w+;)/;
    my $spaces = " "x$nspaces;
    $_ = sprintf("%s%-${max_offset}s", $spaces, $type);
    substr($_, -$nstars, $nstars) = "*"x$nstars.$rest;
  }
}

sub err_exit {
  my $msg = $_[0];
  print STDERR "Error: $msg\n";
  exit 1;
}

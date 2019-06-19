#! /usr/bin/env perl

use 5.014;
use warnings;
use strict;
use Getopt::Long;
use Cwd 'abs_path';
use File::Basename;
use lib abs_path(dirname(__FILE__));
use util;

my $usage = <<EOF;
Usage: $0 [OPTION] ARCH
Generate architecture template code for architecture ARCH.
Print the output code to standard output if --dir and --root are omited.

NOTE:
Remember to put the declaration of your new architecture in ln_arch.c.

Options:
  -h, --help              print this message
  -d, --dir=DIRECTORY     save generated file in DIRECTORY/ln_archimpl_ARCH.c
  -r, --root=ROOT         set project root directory; this option will save
                          generated file in ROOT/src/arch/ln_archimpl_ARCH.c
Author: Zhao Zhixu
EOF

my $root = '';
my $dir = '';
GetOptions(
           'help' => sub {&exit_msg(0, $usage)},
           'dir=s' => \$dir,
           'root=s' => \$root,
          ) or &exit_msg(1, $usage);

&exit_msg(1, $usage) unless @ARGV == 1;
my $arch = shift @ARGV;

my $code_str = <<EOF;
/*
 * Copyright (c) 2018-2019 Zhao Zhixu
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

#include "ln_arch.h"

/* end of declare ${arch} ops */

static ln_op *ops_${arch}[] = {
/* end of init ${arch} ops */
    NULL
};

/* end of declare ${arch} expanders */

ln_expander_func ep_funcs_${arch}[] = {
/* end of ${arch} expanders */
    NULL
};

/* end of declare ${arch} combiners */

ln_combiner_func cb_funcs_${arch}[] = {
/* end of ${arch} combiners */
    NULL
};

/* end of declare ${arch} subgraphers */

ln_subgraph_func sg_funcs_${arch}[] = {
/* end of ${arch} subgraphers */
    NULL
};

/* end of declare ${arch} schedulers */

ln_schedule_func sd_funcs_${arch}[] = {
/* end of ${arch} schedulers */
    NULL
};

/* end of declare ${arch} init funcs */

static void init_${arch}(void **priv_p)
{
/* end of exec ${arch} init funcs */
}

/* end of declare ${arch} cleanup funcs */

static void cleanup_${arch}(void **priv_p)
{
/* end of exec ${arch} cleanup funcs */
}

ln_arch ln_archimpl_${arch} = {
    .init_func = init_${arch},
    .cleanup_func = cleanup_${arch},
    .reg_ops = ops_${arch},
    .ep_funcs = ep_funcs_${arch},
    .cb_funcs = cb_funcs_${arch},
    .sg_funcs = sg_funcs_${arch},
    .sd_funcs = sd_funcs_${arch},
    .arch_name = "${arch}",
};
EOF

if (not $dir and not $root) {
    print $code_str;
}

if ($dir) {
    unless (-d $dir) {
        make_path($dir, {mode => 0755})
            or die "Cannot create directory $dir: $!";
    }
    my $dir_file = "${dir}/ln_archimpl_${arch}.c";
    &backup_write($dir_file, $code_str);
}

if ($root) {
    unless (-d "${root}/src/arch") {
        make_path("${root}/src/arch", {mode => 0755})
            or die "Cannot create directory ${root}/src/arch: $!";
    }
    my $src_file = "${root}/src/arch/ln_archimpl_${arch}.c";
    &backup_write($src_file, $code_str);
}

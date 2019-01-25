#! /usr/bin/env perl

use warnings;
use strict;

my $usage = <<EOF;
Usage: $0 TARGET PREFIX VERSION OUTDIR REQUIRES DESCRIPTION
Generate package information for pkg-config.
TARGET is the library name.
PREFIX is the directory where the library will be installed.
VERSION is the library version.
OUTDIR is the directory where the config file will be generated.
REQUIRES is a quoted requires string for pkg-config
DESCRIPTION is a quoted description string
EOF

if (@ARGV < 6) {
  print $usage;
  exit;
}

my $target = $ARGV[0];
my $prefix = $ARGV[1];
my $version = $ARGV[2];
my $outdir = $ARGV[3];
my $requires = $ARGV[4];
my $description = $ARGV[5];
my $outfile = "$outdir/$target.pc";

my $config_template = <<EOF;
# Package Information for pkg-config

libdir=$prefix/lib
includedir=$prefix/include/$target

Name: $target
Description: $description
Version: $version
Requires: $requires
Libs: -L\$\{libdir\} -l$target
Cflags: -I\$\{includedir\}
EOF

open OUTFILE, '>', $outfile or die "Cannot open $outfile: $!";
print OUTFILE $config_template;
close OUTFILE;

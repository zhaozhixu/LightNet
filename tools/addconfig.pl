#! /usr/bin/env perl

use warnings;
use strict;
use File::Copy;

my $usage = <<EOF;
Usage: $0 FILE DEFINE(s)
Generate c defines in FILE before /* end of config defines */.

Example:
	tools/addconfig.pl build/include/config.h USE_XXX USE_YYY

	This example will generate
	#define USE_XXX
	#define USE_YYY
	before /* end of config defines */ in build/inlcude/config.h.
EOF
if (@ARGV < 1 or $ARGV[0] eq "-h" or $ARGV[0] eq "--help") {
  print $usage;
  exit;
}

my $file = shift @ARGV;
my @defines = @ARGV;

if (@defines == 0) {
  exit;
}

my $config_defines = "";
foreach (@defines) {
  $config_defines .= "#define $_\n";
}

my $bak_file = "$file.bak";
copy($file, $bak_file) or die "Cannot copy $file: $!";
open FILE, '>', $file or die "Cannot open $file: $!";
open BAK_FILE, '<', $bak_file or die "Cannot open $bak_file: $!";
while (<BAK_FILE>) {
  s|/\* end of config defines \*/|$config_defines/* end of config defines */|;
  print FILE;
}
close FILE;
close BAK_FILE;
unlink $bak_file;

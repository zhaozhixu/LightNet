#! /usr/bin/env perl

use 5.014;
use warnings;
use strict;

my $logfile = shift @ARGV;
my $pkgname = shift @ARGV;
my $pyversion = shift @ARGV;
my $prefix = shift @ARGV;
open LOG, '<', $logfile or die "Cannot open log file $logfile: $!";
$_ = <LOG>;
close LOG;
my $re = qr|$prefix/lib/python${pyversion}[0-9a-z.]*/(site\|dist)-packages/$pkgname|;
if (m|$re|) {
    foreach (glob "$&*") {
        say "rm -rf $_";
        !system "rm -rf $_" or die "error removing $_";
    }
} else {
    die "Cannot match installed file $_";
}

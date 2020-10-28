#! /usr/bin/env perl

use warnings;
use strict;

die "$0 requires 2 arguments" unless @ARGV == 2;
my $logfile = shift @ARGV;
my $pkgname = shift @ARGV;
open LOG, '<', $logfile or die "Cannot open log file $logfile: $!";
$_ = <LOG>;
close LOG;
my $re = qr|^.+/(site\|dist)-packages/$pkgname[^/]+|;
if (m|$re|) {
    print "rm -rf $&\n";
    !system "rm -rf $&" or die "error removing $&";
    !system "rm -f $logfile" or die "error removing $logfile";
} else {
    die "Cannot match installed directory $&";
}

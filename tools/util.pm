#! /usr/bin/env perl

package util;

use 5.014;
use warnings;
use strict;

sub err_exit {
    my $msg = $_[0];
    print STDERR "ERROR: $msg\n";
    exit 1;
}

sub warn_msg {
    my $msg = $_[0];
    print STDERR "WARNING: $msg\n";
}

sub exit_msg {
    my $status = shift;
    my $msg = shift;
    print $msg;
    exit $status;
}

1;

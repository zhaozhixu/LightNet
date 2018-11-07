#! /usr/bin/perl

use warnings;
use strict;
use JSON;

my $file = $ARGV[0];
open FILE, '<', $file or die "Cannot op $file: $!";
my $json_text = join '', <FILE>;
close FILE;

my $json = decode_json $json_text;

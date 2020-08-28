#! /usr/bin/env perl

use 5.014;
use warnings;
use strict;
use JSON;
use Getopt::Long;
use Cwd 'abs_path';
use File::Basename;
use lib abs_path(dirname(__FILE__));
use util;

my $usage = <<EOT;
Usage: $0 [Options] WD SRC CMD
Add the compile command CMD of source file SRC relative to working directory WD
to compile command JSON.

Options:
  -h, --help       print this message
  -f, --file=FILE  update JSON in FILE if it is set; or print to stdout
EOT

my $file = '';
GetOptions(
           'help' => sub { exit_msg(0, $usage) },
           'file=s' => \$file,
          ) or exit_msg(1, $usage);

@ARGV == 3 or exit_msg(1, $usage);
my ($wd, $src, $cmd) = @ARGV;

my $cmd_objs = [];
if ($file and -e $file) {
    $cmd_objs = JSON->new->relaxed()->decode(read_file($file));
}

my $found = 0;
foreach my $cmd_obj (@$cmd_objs) {
    if ($cmd_obj->{directory} eq $wd and $cmd_obj->{file} eq $src) {
        $found = 1;
        last if $cmd_obj->{command} eq $cmd;
        say "updating compile command file: $wd/$src";
        $cmd_obj->{command} = $cmd;
        last;
    }
}

if (not $found) {
    push @$cmd_objs, { directory => $wd, file => $src, command => $cmd };
}
my $json_str = JSON->new->pretty()->canonical()->encode($cmd_objs);
if ($file) {
    open CMDFILE, '>', $file or die "Cannot open $file: $!";
    print CMDFILE $json_str;
    close CMDFILE;
} else {
    print $json_str;
}

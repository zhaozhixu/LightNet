#! /usr/bin/env perl

use warnings;
use strict;
use JSON;
use Getopt::Long;
use Fcntl qw(:flock);

my $usage = <<EOT;
Usage: $0 [Options] WD SRC CMD
Add the compile command CMD of source file SRC relative to working directory WD
to compile command JSON.

Options:
  -h, --help       print this message
  -f, --file=FILE  update JSON in FILE if it is set; or print to stdout;
                   FILE must already exists and contains a legal JSON array
EOT

my $file = '';
GetOptions(
           'help' => sub { exit_msg(0, $usage) },
           'file=s' => \$file,
          ) or exit_msg(1, $usage);

@ARGV == 3 or exit_msg(1, $usage);
my ($wd, $src, $cmd) = @ARGV;

my $fh;
my $cmd_objs = [];
if ($file) {
    open $fh, '+<', $file or die "Cannot open ${file}: $!";
    flock($fh, LOCK_EX) or die "Cannot lock ${file}: $!";
    $cmd_objs = JSON->new->relaxed()->decode(join '', <$fh>);
}

my $found = 0;
foreach my $cmd_obj (@$cmd_objs) {
    if ($cmd_obj->{directory} eq $wd and $cmd_obj->{file} eq $src) {
        $found = 1;
        last if $cmd_obj->{command} eq $cmd;
        print "updating compile command file: $wd/$src\n";
        $cmd_obj->{command} = $cmd;
        last;
    }
}

if (not $found) {
    push @$cmd_objs, { directory => $wd, file => $src, command => $cmd };
}
my $json_str = JSON->new->pretty()->canonical()->encode($cmd_objs);
if ($file) {
    truncate($fh, 0) or die "Cannot truncate ${file}: $!";
    seek($fh, 0, 0) or die "Cannot seek ${file}: $!";
    print $fh $json_str;
    flock($fh, LOCK_UN) or die "Cannot unlock ${file}: $!";
    close $fh;
} else {
    print $json_str;
}

sub exit_msg {
    my $status = shift;
    my $msg = shift;
    print $msg;
    exit $status;
}

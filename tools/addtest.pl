#! /usr/bin/perl

use warnings;
use strict;
use File::Copy;
use Cwd 'abs_path';

my $usage = <<EOF;
Usage: $0 ROOT MOD_NAME TEST_NAME(s)
Generate test templates for a module.
ROOT is the path of the project root.
MOD_NAME is the module name.
TEST_NAME is the test name, usually the name of function to be tested.

Example:
	tools/addtest.pl . mod mod_func1 mod_func2

	Executing this example from project root will generate test templates
	test_ln_mod_func1 and test_ln_mod_func2 for module ln_mod in file
 	ROOT/test/test_ln_mod.c
EOF
if (@ARGV < 2) {
  print $usage;
  exit;
}
my $root = abs_path($ARGV[0]);
my $suite_name = $ARGV[1];
my @test_names = @ARGV[2..$#ARGV];

my $tests_str = "";
my $test_add_str = "";
foreach my $test_name (@test_names) {
  $tests_str = $tests_str.<<EOF;

START_TEST(test_ln_${test_name})
{
}
END_TEST
EOF

  $test_add_str = $test_add_str.<<EOF;
     tcase_add_test(tc, test_ln_${test_name});
EOF
}
chomp $tests_str;
chomp $test_add_str;

my $suite_tpl = <<EOF;
/*
 * Copyright (c) 2018-2020 Zhao Zhixu
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

#include <check.h>
#include <tl_check.h>
#include "test_lightnet.h"
#include "../src/ln_${suite_name}.h"

static void checked_setup(void)
{
}

static void checked_teardown(void)
{
}
$tests_str
/* end of tests */

static TCase *make_${suite_name}_tcase(void)
{
     TCase *tc;

     tc = tcase_create("${suite_name}");
     tcase_add_checked_fixture(tc, checked_setup, checked_teardown);

$test_add_str
     /* end of adding tests */

     return tc;
}

void add_${suite_name}_record(test_record *record)
{
    test_record_add_suite(record, "${suite_name}");
    test_record_add_tcase(record, "${suite_name}", "${suite_name}", make_${suite_name}_tcase);
}
EOF

my $test_file = "$root/test/test_ln_${suite_name}.c";
if (-e $test_file) {
  copy($test_file, "$test_file.bak")
    or die "Cannot backup file $test_file: $!";
  open TEST_BAK, '<', "$test_file.bak"
    or die "Cannot open $test_file.bak: $!";
  open TEST, '>', $test_file
    or die "Cannot open $test_file: $!";
  while (<TEST_BAK>) {
    s|/\* end of tests \*/|$tests_str\n/* end of tests */|;
    s|     /\* end of adding tests \*/|$test_add_str\n     /* end of adding tests */|;
    print TEST;
  }
  close TEST;
  close TEST_BAK;
  exit 0;
}
open TEST, '>', $test_file
  or die "Cannot open $test_file: $!";
print TEST $suite_tpl;
close TEST;

my $declare = "void add_${suite_name}_record(test_record *record);";
my $header_file = "$root/test/test_lightnet.h";
copy($header_file, "$header_file.bak")
  or die "Cannot backup file $header_file: $!";
open HEADER_BAK, '<', "$header_file.bak"
  or die "Cannot open $header_file.bak: $!";
open HEADER, '>', $header_file
  or die "Cannot open $header_file: $!";
while (<HEADER_BAK>) {
  s|/\* end of declarations \*/|$declare\n/* end of declarations */|;
  print HEADER;
}
close HEADER;
close HEADER_BAK;

my $adding_suite = "add_${suite_name}_record(record);";
my $main_file = "$root/test/test_lightnet.c";
copy($main_file, "$main_file.bak")
  or die "Cannot backup file $main_file: $!";
open MAIN_BAK, '<', "$main_file.bak"
  or die "Cannot open $main_file.bak: $!";
open MAIN, '>', $main_file
  or die "Cannot open $main_file: $!";
while (<MAIN_BAK>) {
  s|/\* end of adding suites \*/|$adding_suite\n     /* end of adding suites */|;
  print MAIN;
}
close MAIN;
close MAIN_BAK;

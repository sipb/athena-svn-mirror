#!/usr/bin/perl -w

use strict;

# Please note that this script assumes Bourne shell style redirection
# of standard error to standard output: 2>&1
#
# To use this script you will need a compiler that supports
# ISO C99 variadic macros, GNU gcc variadic macros and __func__.
# It was developed with gcc 2.96

# Please note that HAVE_FUNC is only tested in this script with
# C99 variadic macros and both normal logging
# and exceptional logging enabled.


# Test the compiling of a program that calls the
# gok-log.h macros under the 32 possible settings of
#
#   ENABLE_LOGGING_NORMAL
#   ENABLE_LOGGING_EXCEPTIONAL
#   HAVE_ISO_C99_VARIADIC
#   HAVE_GNU_VARIADIC
#   HAVE_FUNC

my $testcode = "test-gok-log-generated-testcode.c";
my $testexecutable = "test-gok-log-generated-testcode";

my ($normal, $exceptional, $c99, $gnu, $func);

for ($normal=0; $normal<2; $normal++) {
    for ($exceptional=0; $exceptional<2; $exceptional++) {
	for ($c99=0; $c99<2; $c99++) {
	    for ($gnu=0; $gnu<2; $gnu++) {
		for ($func=0; $func<2; $func++) {
		    open FH, ">$testcode";
		    write_define("ENABLE_LOGGING_NORMAL", $normal);
		    write_define("ENABLE_LOGGING_EXCEPTIONAL", $exceptional);
		    write_define("HAVE_ISO_C99_VARIADIC", $c99);
		    write_define("HAVE_GNU_VARIADIC", $gnu);
		    write_define("HAVE_FUNC", $func);
		    write_macro_test();
		    close FH;
		    compile_testcode();
		}
	    }
	}
    }
}

# Test the output of a program that has logging calls
# under 4 logging conditions:
#
#   No logging
#   Exceptional logging only
#   Normal Logging only
#   Normal and exceptional logging
#
# And settings of
#
#   HAVE_ISO_C99_VARIADIC
#   HAVE_GNU_VARIADIC
#   HAVE_FUNC

# No logging

for ($c99=0; $c99<2; $c99++) {
    for ($gnu=0; $gnu<2; $gnu++) {
	open FH, ">$testcode";
	write_define("HAVE_ISO_C99_VARIADIC", $c99);
	write_define("HAVE_GNU_VARIADIC", $gnu);
	write_program_with_function_calls();
	close FH;
	compile_testcode();
	my $testoutput = `./$testexecutable 2>&1`;

	test_equals("", $testoutput);
    }
}

# Exceptional logging only

for ($c99=0; $c99<2; $c99++) {
    for ($gnu=0; $gnu<2; $gnu++) {
	open FH, ">$testcode";
	write_define("HAVE_ISO_C99_VARIADIC", $c99);
	write_define("HAVE_GNU_VARIADIC", $gnu);
	write_define("ENABLE_LOGGING_EXCEPTIONAL", 1);
	write_program_with_function_calls();
	close FH;
	compile_testcode();
	my $testoutput = `./$testexecutable 2>&1`;

if ( ($c99 || $gnu) && !($c99 && $gnu) ) { test_equals(<<EOF, $testoutput); }
$testcode:9: X
$testcode:11: X 2 args
$testcode:22: X
$testcode:24: X 2 args
$testcode:34: X
$testcode:36: X 2 args
$testcode:34: X
$testcode:36: X 2 args
EOF

if ( $c99 && $gnu ) { test_equals(<<EOF, $testoutput); }
$testcode:10: X
$testcode:12: X 2 args
$testcode:23: X
$testcode:25: X 2 args
$testcode:35: X
$testcode:37: X 2 args
$testcode:35: X
$testcode:37: X 2 args
EOF

if ( (!$c99) && (!$gnu) ) { test_equals(<<EOF, $testoutput); }
X
X 2 args
X
X 2 args
X
X 2 args
X
X 2 args
EOF

    }
}

# Normal Logging only

for ($c99=0; $c99<2; $c99++) {
    for ($gnu=0; $gnu<2; $gnu++) {
	open FH, ">$testcode";
	write_define("HAVE_ISO_C99_VARIADIC", $c99);
	write_define("HAVE_GNU_VARIADIC", $gnu);
	write_define("ENABLE_LOGGING_NORMAL", 1);
	write_program_with_function_calls();
	close FH;
	compile_testcode();
	my $testoutput = `./$testexecutable 2>&1`;

test_equals(<<EOF, $testoutput);
{
  N
  N 2 args
  {
    N
    N 2 args
    {
      N
      N 2 args
    }
  }
  {
    N
    N 2 args
  }
}
EOF

    }
}

# Normal and exceptional logging

for ($c99=0; $c99<2; $c99++) {
    for ($gnu=0; $gnu<2; $gnu++) {
	open FH, ">$testcode";
	write_define("HAVE_ISO_C99_VARIADIC", $c99);
	write_define("HAVE_GNU_VARIADIC", $gnu);
	write_define("ENABLE_LOGGING_NORMAL", 1);
	write_define("ENABLE_LOGGING_EXCEPTIONAL", 1);
	write_program_with_function_calls();
	close FH;
	compile_testcode();
	my $testoutput = `./$testexecutable 2>&1`;

if ( ($c99 || $gnu) && !($c99 && $gnu) ) { test_equals(<<EOF, $testoutput); }
{
  N
  $testcode:10: X
  N 2 args
  $testcode:12: X 2 args
  {
    N
    $testcode:23: X
    N 2 args
    $testcode:25: X 2 args
    {
      N
      $testcode:35: X
      N 2 args
      $testcode:37: X 2 args
    }
  }
  {
    N
    $testcode:35: X
    N 2 args
    $testcode:37: X 2 args
  }
}
EOF

if ( $c99 && $gnu ) { test_equals(<<EOF, $testoutput); }
{
  N
  $testcode:11: X
  N 2 args
  $testcode:13: X 2 args
  {
    N
    $testcode:24: X
    N 2 args
    $testcode:26: X 2 args
    {
      N
      $testcode:36: X
      N 2 args
      $testcode:38: X 2 args
    }
  }
  {
    N
    $testcode:36: X
    N 2 args
    $testcode:38: X 2 args
  }
}
EOF

if ( (!$c99) && (!$gnu) ) { test_equals(<<EOF, $testoutput); }
{
  N
  X
  N 2 args
  X 2 args
  {
    N
    X
    N 2 args
    X 2 args
    {
      N
      X
      N 2 args
      X 2 args
    }
  }
  {
    N
    X
    N 2 args
    X 2 args
  }
}
EOF

    }
}

# Normal and exceptional logging with HAVE_FUNC

open FH, ">$testcode";
write_define("HAVE_ISO_C99_VARIADIC", 1);
write_define("HAVE_FUNC", 1);
write_define("ENABLE_LOGGING_NORMAL", 1);
write_define("ENABLE_LOGGING_EXCEPTIONAL", 1);
write_program_with_function_calls();
close FH;
compile_testcode();
my $testoutput = `./$testexecutable 2>&1`;

test_equals(<<EOF, $testoutput);
main {
  N
  $testcode:11:main: X
  N 2 args
  $testcode:13:main: X 2 args
  foo {
    N
    $testcode:24:foo: X
    N 2 args
    $testcode:26:foo: X 2 args
    bar {
      N
      $testcode:36:bar: X
      N 2 args
      $testcode:38:bar: X 2 args
    }
  }
  bar {
    N
    $testcode:36:bar: X
    N 2 args
    $testcode:38:bar: X 2 args
  }
}
EOF



sub test_equals {
    my ($expected, $actual) = @_;

    if ($expected ne $actual) {
	print "**** FAIL ****\n";
	print "Expected:\n";
	print "$expected";
	print "Got:\n";
	print "$actual";
	exit 1;
    }
}

sub compile_testcode {
    `gcc -o $testexecutable $testcode gok-log.c`;
    if ($? != 0) {
	print "**** FAIL ****\n";
	exit 1;
    }
}

sub write_define {
    my ($name, $value) = @_;
    if ($value) {
	print FH "#define $name 1\n";
    }
}

sub write_macro_test {
print FH <<EOF;

#include "gok-log.h"

int main () {
    gok_log_enter ();
    gok_log ("N");
    gok_log_x ("X");
    gok_log ("N %d %s", 2, "args");
    gok_log_x ("X %d %s", 2, "args");
    gok_log_leave ();
}
EOF
}

sub write_program_with_function_calls {
print FH <<EOF;

#include "gok-log.h"

int main () {
    gok_log_enter ();
    gok_log ("N");
    gok_log_x ("X");
    gok_log ("N %d %s", 2, "args");
    gok_log_x ("X %d %s", 2, "args");

    foo ();
    bar ();

    gok_log_leave ();
}

int foo () {
    gok_log_enter ();
    gok_log ("N");
    gok_log_x ("X");
    gok_log ("N %d %s", 2, "args");
    gok_log_x ("X %d %s", 2, "args");

    bar ();

    gok_log_leave ();
}

int bar () {
    gok_log_enter ();
    gok_log ("N");
    gok_log_x ("X");
    gok_log ("N %d %s", 2, "args");
    gok_log_x ("X %d %s", 2, "args");
    gok_log_leave ();
}
EOF
}

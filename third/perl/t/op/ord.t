#!./perl

# $RCSfile: ord.t,v $$Revision: 1.1.1.2 $$Date: 1997-11-13 01:47:21 $

print "1..3\n";

# compile time evaluation

if (ord('A') == 65) {print "ok 1\n";} else {print "not ok 1\n";}

# run time evaluation

$x = 'ABC';
if (ord($x) == 65) {print "ok 2\n";} else {print "not ok 2\n";}

if (chr 65 == A) {print "ok 3\n";} else {print "not ok 3\n";}

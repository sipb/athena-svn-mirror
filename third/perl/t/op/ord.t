#!./perl

# $Header: /afs/dev.mit.edu/source/repository/third/perl/t/op/ord.t,v 1.1.1.1 1996-10-02 06:40:15 ghudson Exp $

print "1..2\n";

# compile time evaluation

if (ord('A') == 65) {print "ok 1\n";} else {print "not ok 1\n";}

# run time evaluation

$x = 'ABC';
if (ord($x) == 65) {print "ok 2\n";} else {print "not ok 2\n";}

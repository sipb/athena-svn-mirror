#!./perl

# $Header: /afs/dev.mit.edu/source/repository/third/perl/t/op/sleep.t,v 1.1.1.1 1996-10-02 06:40:15 ghudson Exp $

print "1..1\n";

$x = sleep 2;
if ($x >= 2 && $x <= 10) {print "ok 1\n";} else {print "not ok 1 $x\n";}

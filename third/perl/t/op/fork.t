#!./perl

# $Header: /afs/dev.mit.edu/source/repository/third/perl/t/op/fork.t,v 1.1.1.1 1996-10-02 06:40:15 ghudson Exp $

$| = 1;
print "1..2\n";

if ($cid = fork) {
    sleep 2;
    if ($result = (kill 9, $cid)) {print "ok 2\n";} else {print "not ok 2 $result\n";}
}
else {
    $| = 1;
    print "ok 1\n";
    sleep 10;
}

#!./perl

# $Header: /afs/dev.mit.edu/source/repository/third/perl/t/op/chop.t,v 1.1.1.1 1996-10-02 06:40:15 ghudson Exp $

print "1..4\n";

# optimized

$_ = 'abc';
$c = do foo();
if ($c . $_ eq 'cab') {print "ok 1\n";} else {print "not ok 1 $c$_\n";}

# unoptimized

$_ = 'abc';
$c = chop($_);
if ($c . $_ eq 'cab') {print "ok 2\n";} else {print "not ok 2\n";}

sub foo {
    chop;
}

@foo = ("hi \n","there\n","!\n");
@bar = @foo;
chop(@bar);
print join('',@bar) eq 'hi there!' ? "ok 3\n" : "not ok 3\n";

$foo = "\n";
chop($foo,@foo);
print join('',$foo,@foo) eq 'hi there!' ? "ok 4\n" : "not ok 4\n";

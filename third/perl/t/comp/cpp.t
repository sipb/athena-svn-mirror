#!./perl

# $RCSfile: cpp.t,v $$Revision: 1.1.1.4 $$Date: 2002-02-07 21:12:22 $

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}

use Config;
if ( $^O eq 'MSWin32' or
     ($Config{'cppstdin'} =~ /\bcppstdin\b/) and
     ( ! -x $Config{'binexp'} . "/cppstdin") ) {
    print "1..0 # Skip: \$Config{cppstdin} unavailable\n";
    exit; 		# Cannot test till after install, alas.
}

system "./perl -P comp/cpp.aux"

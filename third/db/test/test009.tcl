# See the file LICENSE for redistribution information.
#
# Copyright (c) 1996, 1997, 1998, 1999, 2000
#	Sleepycat Software.  All rights reserved.
#
#	$Id: test009.tcl,v 1.1.1.2 2002-02-11 16:28:24 ghudson Exp $
#
# DB Test 9 {access method}
# Check that we reuse overflow pages.  Create database with lots of
# big key/data pairs.  Go through and delete and add keys back
# randomly.  Then close the DB and make sure that we have everything
# we think we should.
proc test009 { method {nentries 10000} args} {
	eval {test008 $method $nentries 9 0} $args
}

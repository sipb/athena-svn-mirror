# See the file LICENSE for redistribution information.
#
# Copyright (c) 2000
#	Sleepycat Software.  All rights reserved.
#
#	$Id: test082.tcl,v 1.1.1.1 2002-02-11 16:27:03 ghudson Exp $
#
# Test 82.
# Test of DB_PREV_NODUP
proc test082 { method {dir -prevnodup} {pagesize 512} {nitems 100}\
    {tnum 82} args} {
	source ./include.tcl

	eval {test074 $method $dir $pagesize $nitems $tnum} $args
}

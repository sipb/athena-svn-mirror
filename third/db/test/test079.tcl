# See the file LICENSE for redistribution information.
#
# Copyright (c) 2000
#	Sleepycat Software.  All rights reserved.
#
#	$Id: test079.tcl,v 1.1.1.1 2002-02-11 16:25:31 ghudson Exp $
#
# DB Test 79 {access method}
# Check that delete operations work in large btrees.  10000 entries and
# a pagesize of 512 push this out to a four-level btree, with a small fraction
# of the entries going on overflow pages.
proc test079 { method {nentries 10000} {pagesize 512} {tnum 79} args} {
	eval {test006 $method $nentries 1 $tnum -pagesize $pagesize} $args
}

# See the file LICENSE for redistribution information.
#
# Copyright (c) 2000-2002
#	Sleepycat Software.  All rights reserved.
#
# $Id: test101.tcl,v 1.1.1.1 2004-12-17 17:27:37 ghudson Exp $
#
# TEST	test101
# TEST	Test for functionality near the end of the queue 
# TEST	using test070 (DB_CONSUME).
proc test101 { method {nentries 10000} {txn -txn} {tnum "101"} args} {
	if { [is_queueext $method ] == 0 } {
		puts "Skipping test0$tnum for $method."
		return;
	}
	eval {test070 $method 4 2 1000 WAIT 4294967000 $txn $tnum} $args
}

# See the file LICENSE for redistribution information.
#
# Copyright (c) 1999, 2000
#	Sleepycat Software.  All rights reserved.
#
#	$Id: test071.tcl,v 1.1.1.1 2002-02-11 16:28:26 ghudson Exp $
#
# DB Test 71: Test of DB_CONSUME.
#	This is DB Test 70, with one consumer, one producers, and 10000 items.
proc test071 { method {nconsumers 1} {nproducers 1}\
    {nitems 10000} {tnum 71} args } {

	eval test070 $method $nconsumers $nproducers $nitems $tnum $args
}

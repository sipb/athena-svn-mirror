#!/bin/sh

# This is a generic script for firing up a server, waiting for it to write
# its stringified IOR to a file, then firing up a server

if test "z$ORBIT_TMPDIR" = "z"; then
	ORBIT_TMPDIR="/tmp/orbit-$USER/tst"
	rm -Rf $ORBIT_TMPDIR
	mkdir -p $ORBIT_TMPDIR
fi
TMPDIR=$ORBIT_TMPDIR;
export TMPDIR;

BONOBO_ACTIVATION_SERVER="../../activation-server/bonobo-activation-server";
BONOBO_ACTIVATION_DEBUG_OUTPUT="1";
PATH=".:$PATH";
LD_LIBRARY_PATH="./.libs:$LD_LIBRARY_PATH";

export BONOBO_ACTIVATION_SERVER BONOBO_ACTIVATION_DEBUG_OUTPUT BONOBO_ACTIVATION_PATH PATH LD_LIBRARY_PATH

if ./bonobo-activation-test; then
    exit 0;
else
    exit 1;
fi

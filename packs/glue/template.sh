#!/bin/sh

if [ ! -r /mit/@LOCKER@ ]; then
	/bin/athena/attach -q @LOCKER@
fi
if [ "$ATHENA_ATTACHRUN_RECURSION" = $$ -o ! -x @BINDIR@/@PROGRAM@ ]; then
	echo "Can't find the @PROGRAM@ program in the @LOCKER@ locker."
	echo "Please try again later."
	exit 1
fi
ATHENA_ATTACHRUN_RECURSION=$$ export ATHENA_ATTACHRUN_RECURSION
PATH="@BINDIR@:$PATH"
exec @PROGRAM@ "$@"

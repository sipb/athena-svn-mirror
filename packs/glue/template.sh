#!/bin/sh

if [ ! -r /mit/@LOCKER@ ]; then
	/bin/athena/attach -q @LOCKER@
fi
if [ ! -x @PATH@ ]; then
	echo "Can't find the @PROGRAM@ program in the @LOCKER@ locker."
	echo "Please try again later."
	exit 1
fi
exec @PATH@ "$@"

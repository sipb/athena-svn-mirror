#!/bin/sh

if [ "$ATHENA_ATTACHRUN_RECURSION" = $$ ]; then
	echo "Can't properly run the mkserv program.  Please try again" 1>&2
	echo "later." 1>&2
	exit 1
fi
if /bin/athena/attach -q mkserv; then
	ATHENA_ATTACHRUN_RECURSION=$$ export ATHENA_ATTACHRUN_RECURSION
	bindir=/mit/mkserv/arch/`machtype -S`/bin
	if [ ! -x "$bindir/mkserv" ]; then
		echo "Can't find the mkserv program in the mkserv locker." 1>&2
		echo "Please try again later." 1>&2
		exit 1
	fi
	PATH=${bindir}:$PATH
	exec mkserv "$@"
else
	exit $?
fi

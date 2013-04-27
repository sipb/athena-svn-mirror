#!/bin/sh

SRCDIR=${SRCDIR:-.}

check () {
    if [ "$2" != "$3" ]; then
	echo Regression test failed checking $1:
	echo Expected $2, got $3
	exit 1
    fi;
}

gather () {
    $1 | sort | tr '\012' ' ' | sed -e 's/ $//'
}

check "bitsy's host address" 18.72.0.3 "`$SRCDIR/hostinfo -a bitsy 2>&1`"
check "18.71.0.151's name" STRAWB.MIT.EDU. "`$SRCDIR/hostinfo -h 18.71.0.151 2>&1`"
check "ls's hostinfo" "\"DEC/VAXSTATION-3100\"/\"UNIX\"" "`$SRCDIR/hostinfo -i ls 2>&1`"
check "alum's MX record" alum.mit.edu. "`$SRCDIR/hostinfo -m alum.mit.edu 2>&1`"

athmx=`gather "$SRCDIR/hostinfo -m athena"`
outmx=`gather "$SRCDIR/hostinfo -m outgoing"`
check "athena's MX records match outgoing's MX records" "$athmx" "$outmx"

echo "Regression tests passed."
exit 0

#!/bin/sh

v=`/bin/cat version.c|/bin/sed -n -e '/return/s/^.*\.\([0-9]*\)[^0-9]*$/\1/p'`
x=`/bin/expr $v + 1`

/bin/cp version.c /tmp/version
/bin/sed -n -e '/return/s/^\(.*\.\)'$v'\([^0-9]*\)$/\1'$x'\2/' -e p /tmp/version > version.c
/bin/rm -f /tmp/version

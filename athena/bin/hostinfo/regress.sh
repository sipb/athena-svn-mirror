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
    first=`eval $1`
    list=$first
    count=0
    while [ $count -lt 10 ]; do
	new=`eval $1`
	if [ "$new" = "$first" ]; then
	    break
	fi
	list="$list\n$new"
	count=`expr $count + 1`
    done
    echo $list | sort | tr '\012' ' ' | sed -e 's/ $//'
}

check "bitsy's host address" 18.72.0.3 "`$SRCDIR/hostinfo.pl -a bitsy 2>&1`"
check "18.71.0.151's name" STRAWB.MIT.EDU "`$SRCDIR/hostinfo.pl -h 18.71.0.151 2>&1`"
check "ls's hostinfo" "DEC/VAXSTATION-2000/UNIX" "`$SRCDIR/hostinfo.pl -i ls 2>&1`"
check "ai's MX record" life.ai.mit.edu "`$SRCDIR/hostinfo.pl -m ai.mit.edu 2>&1`"

athmx=`gather "$SRCDIR/hostinfo.pl -m athena"`
mitmx=`gather "$SRCDIR/hostinfo.pl -m mit"`
check "athena's MX records match mit's MX records" "$athmx" "$mitmx"

echo "Regression tests passed."
exit 0

#!/bin/sh
#
# apropos -- search the whatis database for keywords.
# whatis  -- idem, but match only commands (as whole words).
#
# Copyright (c) 1990, 1991, John W. Eaton.
# Copyright (c) 1994-1999, Andries E. Brouwer.
#
# You may distribute under the terms of the GNU General Public
# License as specified in the README file that comes with the man
# distribution.  
#
# apropos/whatis-1.5i aeb 000323 (from man-1.5j)
#
# keep old PATH - 000323 - Bryan Henderson

program=`basename $0`

# Some RedHat versions add the 'a' option to grep (`also search
# binary files'), but this option is understood only by GNU grep,
# and moreover we probably do not want to search binary files.
aproposgrepopt1='ia'
aproposgrepopt2=''
whatisgrepopt1='iwa'
whatisgrepopt2='^'
case $0 in
*apropos)
  grepopt1=$aproposgrepopt1
  grepopt2=$aproposgrepopt2
  ;;
*)
  grepopt1=$whatisgrepopt1
  grepopt2=$whatisgrepopt2
  ;;
esac

if [ $# = 0 ]
then
    echo "usage: $program keyword ..."
    exit 1
fi

manpath=`man --path | tr : '\040'`

if [ "$manpath" = "" ]
then
    echo "$program: manpath is null"
    exit 1
fi

if [ "$PAGER" = "" ]
then
    PAGER="/usr/bin/less -isr"
fi

args=
for arg in $*; do
    case $arg in
        --version|-V|-v)
	    echo "$program from man-1.5j"
	    exit 0
	    ;;
	--help|-h)
            echo "usage: $program keyword ..."
	    exit 0
	    ;;
	-*)
	    echo "$program: $arg: unknown option"
	    exit 1
	    ;;
	*)
	    args="$args $arg"
    esac
done

# avoid using a pager if only output is "nothing appropriate"
nothing=
found=0
while [ $found = 0 -a -n "$1" ]
do
    for d in /var/cache/man $manpath /usr/lib
    do
        if [ -f $d/whatis ]
        then
            if grep -"$grepopt1"q "$grepopt2""$1" $d/whatis > /dev/null
            then
                found=1
            fi
        fi
    done
    if [ $found = 0 ]
    then
	nothing="$nothing $1"
	shift
    fi
done

if [ $found = 0 ]
then
    for i in $nothing
    do
	echo "$i: nothing appropriate"
    done
    exit
fi

while [ $1 ]
do
    for i in $nothing
    do
	echo "$i: nothing appropriate"
    done
    nothing=
    found=0
    for d in /var/cache/man $manpath /usr/lib
    do
        if [ -f $d/whatis ]
        then
            if grep -"$grepopt1" "$grepopt2""$1" $d/whatis
            then
                found=1
            fi
        fi
    done

    if [ $found = 0 ]
    then
        echo "$1: nothing appropriate"
    fi

    shift
done
# Maybe don't use a pager
# | $PAGER

exit

#!/bin/sh
# $Id: clean_tmp_areas.sh,v 1.12 1998-09-22 01:55:09 ghudson Exp $
# Script to clean up some temporary areas in a vaguely general manner.

PATH=/bin/athena:/bin:/usr/bin
export PATH

dirs=/tmp:"-atime +1":/var/tmp:"-atime +2":/var/preserve:"-mtime +3"
xdev=-mount
exceptions="! -type b ! -type c"
args=

case `machtype` in
sun4)
	exceptions="$exceptions ! -type p ! -name ps_data"
	;;
sgi)
	exceptions="$exceptions ! -type p ! -type s ! -user root"
	;;
esac

oldifs="$IFS"
IFS=:
set -- $dirs
IFS="$oldifs"
while [ $# -gt 1 ]; do
	if cd $1; then
		find $args . $xdev $2 $exceptions -exec saferm {} \; -print
		find $args . $xdev -depth ! -name . -type d -mtime +1 -exec saferm -d -q {} \; -print
	fi
	shift 2
done

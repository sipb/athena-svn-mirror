#!/bin/sh
# $Id: lp.sh,v 1.2 1998-09-17 23:13:34 ghudson Exp $

# This script emulates the System V lp command using the Athena lpr
# command.  The emulation is not perfect; the known imperfections are:
#
# 	* The -s option is ignored, since lpr doesn't normally print
#	  output and doesn't have an option to suppress it when it
#	  does.
#
#	* The -m and -w options, which cause mail to be sent or a
#	  message to be written to the user when the print job is
#	  done, are both replaced with -z, which causes a zephyr to be
#	  sent to the user when the print job is done.
#
#	* All -o options are ignored (normally they are
#	  printer-specific options) except for "nobanner".
#
#	* Arguments to some options may be expanded twice for words and
#	  globbing.

# Parse command-line flags.
opts=""
title=""
suppress_s=no
usage="lp [-c] [-d dest] [-m] [-n number] [-t title] [-w] files"
while getopts cd:mn:o:st:w opt; do
	case "$opt" in
	c)
		suppress_s=yes
		;;
	d)
		opts="$opts -P $OPTARG"
		;;
	m)
		opts="$opts -z"
		;;
	n)
		opts="$opts -#$OPTARG"
		;;
	o)
		case "$OPTARG" in
		nobanner|*,nobanner|nobanner,*|*,nobanner,*)
			opts="$opt -h"
			;;
		esac
		;;
	s)
		;;
	t)
		title="$OPTARG"
		;;
	w)
		opts="$opts -z"
		;;
	\?)
		echo "$usage" 1>&2
		exit 1
		;;
	esac
done
if [ "$suppress_s" = no ]; then
	opts="$opts -s"
fi

# Verify that there is at least one file.
shift `expr $OPTIND - 1`
if [ $# -eq 0 ]; then
	echo "$usage" 1>&2
	exit 1
fi

exec /usr/athena/bin/lpr -J "${title:-$@}" $opts "$@"

#!/bin/sh
# $Id: lp.sh,v 1.1 1997-04-04 17:49:29 ghudson Exp $

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
#	  printer-specific options).
#
#	* Arguments to some options may be expanded twice for words and
#	  globbing.

# Parse command-line flags.
opts=""
suppress_s=no
usage="lp [-c] [-ddest] [-m] [-nnumber] [-ooption] [-s] [-ttitle] [-w] files"
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
		;;
	s)
		;;
	t)
		opts="$opts -J $OPTARG"
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

exec /usr/athena/bin/lpr $opts "$@"

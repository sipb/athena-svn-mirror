#!/bin/sh
# $Id: lp.sh,v 1.4 1998-09-29 17:20:40 ghudson Exp $

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
suppress_s=no
content=""
pages=""
title=""
usage="lp [-P pagerange] [-T content-type] [-c] [-d dest] [-m] [-n number]
   [-o option] [-t title] [-w] [file ...]"
while getopts H:P:S:T:cd:fmn:o:pq:rst:wy: opt; do
	case "$opt" in
	H)
		echo "lp: Spooling system does not support special handling." \
			1>&2
		;;
	P)
		pages="$OPTARG"
		;;
	S)
		echo "lp: Spooling system does not support print wheels." 1>&2
		;;
	T)
		content="$OPTARG"
		;;
	c)
		suppress_s=yes
		;;
	d)
		opts="$opts -P $OPTARG"
		;;
	f)
		echo "lp: Spooling system does not support forms." 1>&2
		exit 1
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
	p|q|r|s)
		;;
	t)
		title="$OPTARG"
		;;
	w)
		opts="$opts -z"
		;;
	y)
		echo "lp: Spooling system does not support mode list." 1>&2
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

# Find filter to use for content type, if any.
filter=""
case ${content:-simple} in
troff)
	filter=/usr/lib/lp/postscript/dpost
	;;
daisy|dmd|plot)
	filter=/usr/lib/lp/postscript/post$content
	;;
matrix)
	filter=/usr/lib/lp/postscript/postmd
	;;
tek4014)
	filter=/usr/lib/lp/postscript/posttek
	;;
simple|postscript)
	;;
*)
	echo "lp: Spooling system does not support content-type" \
		" $content" 1>&2
	;;
esac

# If a page range was specified, see if the filter supports it, and
# switch to using a filter if the content type is simple or
# postscript.  Page ranges won't work if no content type is specified,
# since we don't know if files are text or postscript and there's no
# filter which does automatic recognition.
case "$pages" in
"")
	;;
*)
	case "$content" in
	troff|daisy|dmd|matrix|plot|tek4014)
		filter="$filter -o$pages"
		;;
	simple)
		filter="/usr/lib/lp/postscript/postprint -o$pages"
		;;
	postscript)
		filter="/usr/lib/lp/postscript/postpages -o$pages"
		;;
	*)
		echo "lp: Spooling system does not support page ranges." 1>&2
		exit
		;;
	esac
esac

shift `expr $OPTIND - 1`

if [ -n "$filter" ]; then
	# This option may not work with all lpr options.
	for file in "${@:--}"; do
		$filter $file | lpr -J "${title:-$file}" $opts
	done
else
	exec /usr/athena/bin/lpr -J "${title:-$@}" $opts "$@"
fi

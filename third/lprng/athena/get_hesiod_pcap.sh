#!/bin/sh
# $Id: get_hesiod_pcap.sh,v 1.1 1999-02-25 21:42:14 danw Exp $

# Support script used by LPRng to fetch Hesiod printcap entries

PATH=/bin/athena:/usr/bin:/usr/bsd

read printer

case "$printer" in
all)
    # If printer is "all", it wants a list of all available printers, the
    # first of which will be considered the default printer. We can't get
    # all printers, so just give the default.

    if [ -z "$PRINTER" ]; then
	host=`hostname`
	PRINTER=`hesinfo $host cluster 2>/dev/null | \
	    awk '/^lpr / {print $2; exit;}'`
	if [ -z "$PRINTER" ]; then
	    echo "No default printer for $host" 1>&2
	    exit 1
	fi
    fi
    echo "all:all=$PRINTER"
    ;;

*)
    # Otherwise just look up the printer it asked for
    hesinfo $printer pcap 2>/dev/null
    ;;
esac

#!/bin/sh -
#
# $Id: config_afs.sh,v 1.7 1992-08-14 12:55:18 probe Exp $
#
# This script configures the workstation's notion of AFS.
# 1. It updates the cell location information from /usr/vice/etc/CellServDB
# 2. It updates the cell setuid information from /usr/vice/etc/SuidCells
PATH=/bin:/bin/athena; export PATH

VICEDIR=/usr/vice/etc
CELLDB=${VICEDIR}/CellServDB
SUIDDB=${VICEDIR}/SuidCells

echo "Updating cell location information"
rm -f ${VICEDIR}/Ctmp
cp /afs/athena.mit.edu/service/CellServDB ${VICEDIR}/Ctmp && \
	[ -s ${VICEDIR}/Ctmp ] && \
	mv -f ${VICEDIR}/Ctmp ${CELLDB}.public && \
	cat ${CELLDB}.public ${CELLDB}.local >${VICEDIR}/Ctmp 2>/dev/null
mv -f ${VICEDIR}/Ctmp ${CELLDB}

/bin/awk ' \
	  /^>/ {printf("\nfs newcell %s", substr($1,2,length($1)-1))}; \
	  /^[0-9]/ {printf(" %s",$1)}; \
	  END {printf("\n")}' ${CELLDB} | \
	/bin/sh

echo "Updating setuid cell information"
rm -f ${VICEDIR}/Ctmp
cp /afs/athena.mit.edu/service/SuidCells ${VICEDIR}/Ctmp && \
	[ -s ${VICEDIR}/Ctmp ] && \
	mv -f ${VICEDIR}/Ctmp ${SUIDDB}.public && \
	cat ${SUIDDB}.public ${SUIDDB}.local >${VICEDIR}/Ctmp 2>/dev/null
mv -f ${VICEDIR}/Ctmp ${SUIDDB}

echo "Only allowing setuid/setgid programs from the following cells:"

(awk '/^>/ {print $1}' ${CELLDB}; cat ${SUIDDB}) | awk '\
	/^>/ {i++; cells[i]=substr($1,2,length($1)-1);suid[i]=0;next}; \
	/^-/ {for (j=1;j<i;j++) {if (substr($1,2,length($1)-1)==cells[j]) \
		{suid[j]=0;next;}}}; \
	{for (j=1;j<i;j++) {if ($1==cells[j]) {suid[j]=1;next}}}; \
	END {	ns=0; nn=0; \
		for (j=1;j<i;j++) { \
		  if (suid[j]){ns++;scmd=scmd" "cells[j];\
				print "echo",cells[j];}\
		  else {nn++;ncmd=ncmd" "cells[j];}; } \
		if (ns) {printf("fs setcell %s -suid\n", scmd)};\
		if (nn) {printf("fs setcell %s -nosuid\n",ncmd)};\
	}' | sh

exit 0

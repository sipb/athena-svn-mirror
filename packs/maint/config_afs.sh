#!/bin/sh -
#
# $Id: config_afs.sh,v 1.12 1996-05-15 19:02:22 ghudson Exp $
#
# This script configures the workstation's notion of AFS.
# 1. It updates the cell location information from /usr/vice/etc/CellServDB
# 2. It updates the cell setuid information from /usr/vice/etc/SuidCells
PATH=/bin:/bin/athena:/usr/bin; export PATH

VICEDIR=/usr/vice/etc
CELLDB=${VICEDIR}/CellServDB
SUIDDB=${VICEDIR}/SuidCells

echo "Updating cell location information"
rm -f ${VICEDIR}/Ctmp
cp /afs/athena.mit.edu/service/CellServDB ${VICEDIR}/Ctmp && \
	[ -s ${VICEDIR}/Ctmp ] && \
	mv -f ${VICEDIR}/Ctmp ${CELLDB}.public && \
	cat ${CELLDB}.public ${CELLDB}.local >${VICEDIR}/Ctmp 2>/dev/null
rm -f ${CELLDB}.last
ln ${CELLDB} ${CELLDB}.last
mv -f ${VICEDIR}/Ctmp ${CELLDB}
chmod 644 ${CELLDB}

cmp -s ${CELLDB}.last ${CELLDB} || \
awk ' \
	  /^>/ {printf("\nfs newcell %s", substr($1,2,length($1)-1))}; \
	  /^[0-9]/ {printf(" %s",$1)}; \
	  END {printf("\n")}' ${CELLDB} | sh

echo "Updating setuid cell information"
rm -f ${VICEDIR}/Ctmp
cp /afs/athena.mit.edu/service/SuidCells ${VICEDIR}/Ctmp && \
	[ -s ${VICEDIR}/Ctmp ] && \
	mv -f ${VICEDIR}/Ctmp ${SUIDDB}.public && \
	cat ${SUIDDB}.public ${SUIDDB}.local >${VICEDIR}/Ctmp 2>/dev/null
mv -f ${VICEDIR}/Ctmp ${SUIDDB}
chmod 644 ${SUIDDB}

echo "Only allowing setuid/setgid programs from the following cells:"

/etc/athena/listsuidcells | xargs -icellname fs setcell cellname -nosuid
cat ${SUIDDB} | awk '
	/^-/	{ suid[substr($1,2,length($1)-1)] = 0; }
	/^[^-]/	{ suid[$1] = 1;
		  cells[numcells++] = $1; }
	END	{ for (i = 0; i < numcells; i++) {
			if (suid[cells[i]]) {
				printf("fs setcell %s -suid\n", cells[i]);
				printf("echo %s\n", cells[i]); } } }' | sh

exit 0

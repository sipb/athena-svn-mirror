#!/bin/sh -
#
# $Id: config_afs.sh,v 1.3 1991-07-20 14:33:13 epeisach Exp $
#
# This script configures the workstation's notion of AFS.
# 1. It updates the cell location information from /usr/vice/etc/CellServDB
# 2. It updates the cell setuid information from /usr/vice/etc/SuidCells
PATH=/bin:/bin/athena; export PATH

VICEDIR=/usr/vice/etc
CELLDB=${VICEDIR}/CellServDB
SUIDFILE=${VICEDIR}/SuidCells

echo "Updating cell location information"
rm -f ${VICEDIR}/Ctmp
cp /afs/athena.mit.edu/service/CellServDB ${VICEDIR}/Ctmp && \
	mv -f ${VICEDIR}/Ctmp ${CELLDB}.public && \
	cat ${CELLDB}.public ${CELLDB}.local >${VICEDIR}/Ctmp 2>/dev/null
mv -f ${VICEDIR}/Ctmp ${CELLDB}

/bin/awk ' \
	  /^>/ {printf("\nfs newcell %s", substr($1,2,length($1)-1))}; \
	  /^[0-9]/ {printf(" %s",$1)}; \
	  END {printf("\n")}' ${CELLDB} | \
	/bin/sh

if [ ! -f ${SUIDFILE} ]; then
    echo "${SUIDFILE} does not exist; allowing setuid programs from all cells:"
    STATUS="-suid"
else
    echo "Disallowing setuid/setgid programs from the following cells:"
    STATUS="-nosuid"
fi

tmp="`awk '/^>/ {print substr($1,2,length($1)-1)}' ${CELLDB}`"

if [ ! -f ${SUIDFILE} -o ! -s ${SUIDFILE} ]; then
    echo "$tmp"
    fs setcell $tmp $STATUS
else
    suid_cells="`cat ${SUIDFILE}`"
    cells_sed="`echo $suid_cells | \
	awk '{for (i=1;i<=NF;i++) {printf(\"-e /^%s$/d \",$i)}}'`"
    cells="`echo "$tmp"|sed -n $cells_sed -e p`"
    if [ "$cells" != "" ]; then
        echo "$cells"
        fs setcell $cells $STATUS
    fi
    echo "
Allowing setuid/setgid programs from only the following cells:
$suid_cells"
    fs setcell $suid_cells -suid
fi
exit 0

#!/bin/sh -
#
# $Id: config_afs.sh,v 1.19 2002-02-15 19:39:50 zacheiss Exp $
#
# This script configures the workstation's notion of AFS.
# 1. It updates the cell location information from /usr/vice/etc/CellServDB
# 2. It updates the cell setuid information from /usr/vice/etc/SuidCells
# 3. It updates the cell alias information from /usr/vice/etc/CellAlias
PATH=/bin:/bin/athena:/usr/bin; export PATH

VICEDIR=/usr/vice/etc
CELLDB=${VICEDIR}/CellServDB
SUIDDB=${VICEDIR}/SuidCells
ALIAS=${VICEDIR}/CellAlias
HOSTTYPE=`/bin/athena/machtype`

echo "Updating cell location information"
rm -f ${VICEDIR}/Ctmp
cp /afs/athena.mit.edu/service/CellServDB ${VICEDIR}/Ctmp &&
	[ -s ${VICEDIR}/Ctmp ] &&
	mv -f ${VICEDIR}/Ctmp ${CELLDB}.public &&
	cat ${CELLDB}.public ${CELLDB}.local >${VICEDIR}/Ctmp 2>/dev/null
[ -s ${VICEDIR}/Ctmp ] && {
	cmp -s ${VICEDIR}/Ctmp ${CELLDB} || {
		rm -f ${CELLDB}.last &&
		ln ${CELLDB} ${CELLDB}.last &&
		mv -f ${VICEDIR}/Ctmp ${CELLDB} &&
		chmod 644 ${CELLDB} &&
		sync &&
		awk '
			/^>/ {printf("\nfs newcell %s", \
				substr($1,2,length($1)-1))};
			/^[0-9]/ {printf(" %s",$1)};
			END {printf("\n")}' ${CELLDB} | sh
	}
}

echo "Updating setuid cell information"
rm -f ${VICEDIR}/Ctmp
cp /afs/athena.mit.edu/service/SuidCells ${VICEDIR}/Ctmp &&
	[ -s ${VICEDIR}/Ctmp ] &&
	mv -f ${VICEDIR}/Ctmp ${SUIDDB}.public &&
	cat ${SUIDDB}.public ${SUIDDB}.local >${VICEDIR}/Ctmp 2>/dev/null
[ -s ${VICEDIR}/Ctmp ] &&
	mv -f ${VICEDIR}/Ctmp ${SUIDDB} &&
	chmod 644 ${SUIDDB}

echo "Only allowing setuid/setgid programs from the following cells:"

for cellname in `/etc/athena/listsuidcells`; do
	fs setcell "$cellname" -nosuid
done
cat ${SUIDDB} | awk '
	/^-$/	{ numcells = 0; exit; }
	/^-/	{ suid[substr($1,2,length($1)-1)] = 0; }
	/^[^-]/	{ suid[$1] = 1;
		  cells[numcells++] = $1; }
	END	{ for (i = 0; i < numcells; i++) {
			if (suid[cells[i]]) {
				printf("fs setcell %s -suid\n", cells[i]);
				printf("echo %s\n", cells[i]); } } }' | sh


if [ sgi != "$HOSTTYPE" ]; then
    echo "Updating cell alias information"
    rm -f ${VICEDIR}/Ctmp
    cp /afs/athena.mit.edu/service/CellAlias ${VICEDIR}/Ctmp &&
	    [ -s ${VICEDIR}/Ctmp ] &&
	    mv -f ${VICEDIR}/Ctmp ${ALIAS}.public &&
	    cat ${ALIAS}.public ${ALIAS}.local >${VICEDIR}/Ctmp 2>/dev/null
    [ -s ${VICEDIR}/Ctmp ] && {
	    cmp -s ${VICEDIR}/Ctmp ${ALIAS} || {
		    rm -f ${ALIAS}.last &&
		    mv -f ${VICEDIR}/Ctmp ${ALIAS} &&
		    chmod 644 ${ALIAS} &&
		    sync &&
		    awk '
			    /^#/ {next} \
			    NF == 2 && {print "fs newalias",$2,$1}' ${ALIAS} \
			| sh
	    }
    }
fi

exit 0

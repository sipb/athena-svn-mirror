#!/bin/sh
# Get cluster information from Hesiod and save it.
# This is normally exected by /etc/rc and each time a workstation is
# activated.
#
# $Id: save_cluster_info.solaris.sh,v 1.1 1993-10-12 06:03:02 probe Exp $
#
# Errors from getcluster guarantee that stdout will be size 0,
# therefore a size # 0 is a sufficient test.
# Use old data from last session if getcluster fails.

PATH=/bin:/usr/ucb; export PATH

. /etc/athena/rc.conf
HOSTNAME=`uname -n`
VERSION=`awk '{vers = $5}; END {print vers}' /etc/athena/version`

if [ "${VERSION}" = "Update" ]; then
	echo "This system is in the middle of an update."
	echo "Please contact Athena Operations."
	exit 1
fi

/bin/athena/getcluster -b ${HOSTNAME} ${VERSION} > /tmp/clusterinfo.bsh
if [ -s /tmp/clusterinfo.bsh ]
then
	cp /tmp/clusterinfo.bsh /etc/athena/clusterinfo.bsh
	chmod 666 /etc/athena/clusterinfo.bsh 2>/dev/null
fi
/bin/athena/getcluster ${HOSTNAME} ${VERSION} > /tmp/clusterinfo
if [ -s /tmp/clusterinfo ]
then
	cp /tmp/clusterinfo /etc/athena/clusterinfo
	chmod 666 /etc/athena/clusterinfo 2>/dev/null
fi

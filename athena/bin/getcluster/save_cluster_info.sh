#!/bin/sh
# Get cluster information from Hesiod and save it.
# This is normally exected by /etc/rc and each time a workstation is
# activated.
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/getcluster/save_cluster_info.sh,v $
#	$Author: lwvanels $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/getcluster/save_cluster_info.sh,v 1.5 1991-07-14 15:29:30 lwvanels Exp $
#
# Errors from getcluster guarantee that stdout will be size 0,
# therefore a size # 0 is a sufficient test.
# Use old data from last session if getcluster fails.

HOSTNAME=`/bin/hostname`
VERSION=`/bin/awk '{vers = $5} END {print vers}' /etc/athena/version`

if [ ${VERSION}x = Updatex ]; then
	echo "This system is in the middle of an update."
	echo "Please contact Athena Operations."
	exit 1
fi

/bin/athena/getcluster -b $HOSTNAME $VERSION > /tmp/clusterinfo.bsh
if [ -s /tmp/clusterinfo.bsh ]
then
	/bin/cp /tmp/clusterinfo.bsh /etc/athena/clusterinfo.bsh
	chmod 666 /etc/athena/clusterinfo.bsh 2>/dev/null
fi
/bin/athena/getcluster $HOSTNAME $VERSION > /tmp/clusterinfo
if [ -s /tmp/clusterinfo ]
then
	/bin/cp /tmp/clusterinfo /etc/athena/clusterinfo
	chmod 666 /etc/athena/clusterinfo 2>/dev/null
fi

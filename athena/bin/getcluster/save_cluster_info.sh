#!/bin/sh
# Get cluster information from Hesiod and save it.
# This is normally exected by /etc/rc and each time a workstation is
# activated.
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/getcluster/save_cluster_info.sh,v $
#	$Author: treese $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/getcluster/save_cluster_info.sh,v 1.4 1987-08-22 17:42:07 treese Exp $
#
# Errors from getcluster guarantee that stdout will be size 0,
# therefore a size # 0 is a sufficient test.
# Use old data from last session if getcluster fails.

HOSTNAME=`/bin/hostname`
VERSION=`/bin/awk '{vers = $5} END {print vers}' /etc/version`

if [ ${VERSION}x = Updatex ]; then
	echo "This system is in the middle of an update."
	echo "Please contact Athena Operations."
	exit 1
fi

/bin/athena/getcluster -b $HOSTNAME $VERSION > /tmp/clusterinfo.bsh
if [ -s /tmp/clusterinfo.bsh ]
then
	/bin/cp /tmp/clusterinfo.bsh /etc/clusterinfo.bsh
	chmod 666 /etc/clusterinfo.bsh 2>/dev/null
fi
/bin/athena/getcluster $HOSTNAME $VERSION > /tmp/clusterinfo
if [ -s /tmp/clusterinfo ]
then
	/bin/cp /tmp/clusterinfo /etc/clusterinfo
	chmod 666 /etc/clusterinfo 2>/dev/null
fi

#!/bin/sh
# $Id: do-update.sh,v 1.34 2000-03-20 20:34:54 rbasch Exp $

# Copyright 1996 by the Massachusetts Institute of Technology.
#
# Permission to use, copy, modify, and distribute this
# software and its documentation for any purpose and without
# fee is hereby granted, provided that the above copyright
# notice appear in all copies and that both that copyright
# notice and this permission notice appear in supporting
# documentation, and that the name of M.I.T. not be used in
# advertising or publicity pertaining to distribution of the
# software without specific, written prior permission.
# M.I.T. makes no representations about the suitability of
# this software for any purpose.  It is provided "as is"
# without express or implied warranty.

# Eliminate duplicates in the argument list, putting the result into
# a variable "out".  Not super-robust, but it's only intended to work
# on filenames added to CONFCHG.  We have this because we can't rely
# on "sort" working across OS versions.
undup() {
	for i in "$@"; do
		case $out in
		*" $i "*)
			;;
		*)
			out="$out $i "
			;;
		esac
	done
}

echo "Starting update at `date`."
. /srvd/usr/athena/lib/update/update-environment

# Get the platform name for Solaris.  "uname -i" is the documented way, but
# it doesn't work in Solaris 2.4 and prior, and "uname -m" works for now.
# Also determine the root disk based on the mount table.
case "$HOSTTYPE" in
sun4)
	SUNPLATFORM=`uname -m`
	ROOTDISK=`mount | sed -n '/^\/ /s#^/ on /dev/dsk/\([^ ]*\).*$#\1#p'`
	export SUNPLATFORM ROOTDISK
	;;
esac

# We get three arguments: the method by which the machine was rebooted.
# (possible values are Auto, Manual, and Remote), the current workstation
# version, and the new version.
method=$1
version=$2
newvers=$3

# On IRIX, reboot prompts for confirmation when logged in remotely.
# The following kludge prevents the prompt.
unset REMOTEHOST

# Some versions of Solaris will panic if they are hit with a certain
# type of port scan and you then kill a network daemon.  As a
# work-around, sync the disks first, and sleep to give the machine a
# chance to panic if it is going to.
sync
sleep 2
# We could be more intelligent and shutdown everything, but...
echo "Shutting down running services"
if [ -f /var/athena/inetd.pid ]; then
	kill `cat /var/athena/inetd.pid` > /dev/null 2>&1
fi
if [ -f /var/athena/sshd.pid ]; then
	kill `cat /var/athena/sshd.pid` > /dev/null 2>&1
fi
if [ -f /var/athena/named.pid ]; then
	kill `cat /var/athena/named.pid` > /dev/null 2>&1
fi
case "$HOSTTYPE" in
sgi)
	killall inetd snmpd syslogd
	;;
*)
	if [ -f /etc/syslog.pid ]; then
		kill `cat /etc/syslog.pid` > /dev/null 2>&1
	fi
	if [ -f /var/athena/snmpd.pid ]; then
		kill `cat /var/athena/snmpd.pid` > /dev/null 2>&1
	fi
	;;
esac
sleep 10

echo "Athena Workstation ($HOSTTYPE) Version Update `date`" >> \
	"$CONFDIR/version"

if [ "$version" != "$newvers" ]; then
	echo "Version-specific updating.."
	cp /dev/null "$CONFCHG"
	cp /dev/null "$CONFVARS"
	cp /dev/null "$OLDBINS"
	cp /dev/null "$OLDLIBS"
	upvers "$version" "$newvers" "$LIBDIR"
fi

. $CONFDIR/rc.conf
if [ -f $CONFVARS ]; then
	. $CONFVARS
fi

# The version scripts may have modified the following files:
#	$CONFCHG	A list of configuration files to update
#	$AUXDEVS	A list of auxiliary device scripts to run
#			from /srvd/install/aux.devs (sun4 only)
#	$OLDBINS	A list of binaries to preserve before tracking
#	$OLDLIBS	A list of libraries to preserve before tracking
#	$DEADFILES	A list of local files to be removed
#	$LOCALPACKAGES	A list of local OS packages to be de/installed
#	$LINKPACKAGES	A list of linked OS packages to be de/installed
#	$CONFIGVERS	A list of new/old versions of config files,
#			left behind by OS installation (Irix only)
#	$CONFVARS	Can set variables to "true", including:
#		NEWUNIX		Update kernel
#		NEWBOOT		Boot blocks have changed
#		NEWDEVS		New pseudo-devices required
#		NEWOS		OS version has changed
#		TRACKOS		OS files relevant to local disk have changed
#		MINIROOT	some OS files require a miniroot update

configfiles=`cat $LIBDIR/configfiles`

if [ "$PUBLIC" = true ]; then
	NEWUNIX=true
	NEWBOOT=true
	for i in $configfiles; do
		echo "$i" >> $CONFCHG;
	done
	rm -f /.hushlogin /etc/*.local /etc/athena/*.local
fi
for i in $configfiles; do
	if [ ! -f "$i" ]; then
		echo "$i" >> $CONFCHG
	fi
done

if [ -s "$CONFCHG" ]; then
	undup `cat "$CONFCHG"`
	conf=$out
	if [ "$PUBLIC" != true ]; then
		echo "The following configuration files have changed and will"
		echo "be replaced.  The old versions will be renamed to the"
		echo "same name, but with a .old extension.  For example,"
		echo "/etc/shells would be renamed to /etc/shells.old and a"
		echo "new version would take its place."
		echo ""
		for i in $conf; do
			if [ -f $i ]; then
				echo "        $i"
			fi
		done
		echo ""
		echo "Press return to continue"
		read foo
		for i in $conf; do
			if [ -f $i ]; then
				mv -f $i $i.old
			fi
		done
	fi
	for i in $conf; do
		rm -rf $i
		if [ -f /srvd$i ]; then
			cp -p /srvd$i $i
		elif [ -f /os$i ]; then
			cp -p /os$i $i
		elif [ -f /install$i ]; then
			cp -p /install$i $i
		fi
	done
fi

if [ "$PUBLIC" = true ]; then
	# Just substitute who we are into the current rc.conf from the srvd.
	echo "Updating $CONFDIR/rc.conf from /srvd$CONFDIR/rc.conf"
	sed -n	-e "s#^HOST=[^;]*#HOST=$HOST#" \
		-e "s#^ADDR=[^;]*#ADDR=$ADDR#" \
		-e "s#^MACHINE=[^;]*#MACHINE=$MACHINE#" \
		-e "s#^SYSTEM=[^;]*#SYSTEM=$SYSTEM#" \
		-e "s#^NETDEV=[^;]*#NETDEV=$NETDEV#" \
		-e p "/srvd$CONFDIR/rc.conf" > "$CONFDIR/rc.conf"
else
	# Add any new variables to rc.conf.
	echo "Looking for new variables to add to $CONFDIR/rc.conf"
	conf=`cat "/srvd$CONFDIR/rc.conf" | awk -F= '(NF>1){print $1}'`
	vars=""
	for i in $conf; do
		if [ `grep -c "^$i=" "$CONFDIR/rc.conf"` = 0 ]; then 
			vars="$vars $i"
		fi
	done
	if [ -n "$vars" ]; then
		echo "Backing up $CONFDIR/rc.conf to $CONFDIR/rc.conf.orig"
		rm -f "$CONFDIR/rc.conf.orig"
		cp -p "$CONFDIR/rc.conf" "$CONFDIR/rc.conf.orig"

		echo "The following variables are being added to /etc/rc.conf:"
		echo "	$vars"
		for i in $vars; do
			grep "^$i=" "/srvd$CONFDIR/rc.conf" \
				>> "$CONFDIR/rc.conf"
		done
	fi
fi

# MINIROOT is currently only used for Irix 6.x.
if [ "$MINIROOT" = true ]; then
	# Set up a miniroot in the swap partition. We will boot into
	# it, and update-os will be run from there.

	echo "Suppressing network daemons for reboot"
	chkconfig -f suppress-network-daemons on

	# Note the volume header must be updated before the miniroot
	# can boot (Irix only).
	if [ "$NEWBOOT" = true ]; then
		# Make sure the volume header has an up-to-date sash.
		echo "Updating sash volume directory entry..."
		dvhtool -v creat /install/lib/sash sash
	fi

	sh /srvd/usr/athena/lib/update/setup-swap-boot "$method" "$newvers"
	case "$?" in
	0)
		echo "Rebooting into swap to update OS files..."
		;;
	2)
		echo "Rebooting to clear swap..."
		echo "Athena Workstation ($HOSTTYPE) Version" \
                      "ClearSwap $method $newvers `date`" \
			>> "$CONFDIR/version"
		# Make sure this machine knows what the heck to do
		# with "ClearSwap" in version, since it may not be
		# updated to a release that understands it yet.
		cp /srvd/etc/init.d/finish-update /etc/init.d
		;;
	*)
		echo "Please contact Athena Hotline at x3-1410."
		echo "Thank you. -Athena Operations"
		exit 0
		;;
	esac

	echo "Update partially completed, system will reboot in 15 seconds."
	sync
	sleep 15
	exec reboot
fi

# Not a miniroot update; run update-os here.  This is a good place to
# handle OLDBINS and OLDLIBS, since we need the environment variables
# after update-os.

if [ -s "$OLDBINS" ]; then
	echo "Making copies of OS binaries we need"
	mkdir -p /tmp/bin
	bins="`cat $OLDBINS`"
	for i in $bins; do
		cp -p $i /tmp/bin/`basename $i`
	done
	PATH=/tmp/bin:$PATH; export PATH
fi

if [ -s "$OLDLIBS" ]; then
	echo "Making copies of OS libraries we need"
	mkdir -p /tmp/lib
	libs="`cat $OLDLIBS`"
	for i in $libs; do
		cp -p $i /tmp/lib/`basename $i`
	done
	LD_LIBRARY_PATH=/tmp/lib; export LD_LIBRARY_PATH
fi

sh /srvd/usr/athena/lib/update/update-os

if [ "$NEWOS" = true ]; then
	# Reboot to finish update.

	case "$HOSTTYPE" in
	sgi)
		echo "Suppressing network daemons for reboot"
		chkconfig -f suppress-network-daemons on
		;;
	esac

	echo "Updating version for reboot"
	echo "Athena Workstation ($HOSTTYPE) Version Reboot" \
		"$method $newvers `date`" >> "$CONFDIR/version"
	echo "Update partially completed, system will reboot in 15 seconds."
	sync
	sleep 15
	exec reboot
fi

# Not an OS update; run finish-update here.  Restart named first, since
# mkserv has to resolve names.
/etc/athena/named
sh /srvd/usr/athena/lib/update/finish-update "$newvers"
if [ "$method" = Auto ]; then
	echo "Automatic update done; system will reboot in 15 seconds."
	sync
	sleep 15
	exec reboot
fi

# Not an automatic update; just sync the disk and drop back to the shell.
sync

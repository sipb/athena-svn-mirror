#!/bin/sh
# $Id: do-update.sh,v 1.17.2.3 1998-02-26 23:33:54 cfields Exp $

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

. /srvd/usr/athena/lib/update/update-environment

# Get the platform name for Solaris.  "uname -i" is the documented way, but
# it doesn't work in Solaris 2.4 and prior, and "uname -m" works for now.
case "$HOSTTYPE" in
sun4)
	platform=`uname -m`
	;;
esac

# We get one argument, the method by which the machine was rebooted.
# Possible values are Auto, Manual, and Remote.
method=$1

echo "Starting update"

echo "Athena Workstation ($HOSTTYPE) Version Update `date`" >> \
	"$CONFDIR/version"

if [ "$VERSION" != "$NEWVERS" ]; then
	echo "Version-specific updating.."
	cp /dev/null $CONFCHG
	cp /dev/null $CONFVARS
	cp /dev/null $OLDBINS
	upvers "$VERSION" "$NEWVERS" "$LIBDIR"
fi

. $CONFDIR/rc.conf
if [ -f $CONFVARS ]; then
	. $CONFVARS
fi

# The version scripts may have modified the following files:
#	$CONFCHG	A list of configuration files to update
#	$AUXDEVS	A list of auxiliary device scripts to run
#			from /srvd/install/aux.devs (sun4 only)
#	$OLDBINS	A list of binaries to preserve before
#			tracking
#	$DEADFILES	A list of local files to be removed
#	$LOCALPACKAGES	A list of local OS packages to be de/installed
#	$LINKPACKAGES	A list of linked OS packages to be de/installed
#	$CONFVARS	Can set variables to "true", including:
#		NEWUNIX		Update kernel
#		NEWBOOT		Boot blocks have changed
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
	touch /etc/named.local
fi
for i in $configfiles; do
	if [ ! -f "$i" ]; then
		echo "$i" >> $CONFCHG
	fi
done

if [ -s "$CONFCHG" ]; then
	conf="`sort -u $CONFCHG`"
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
	sed -n	-e "s/^HOST=[^;]*/HOST=$HOST/" \
		-e "s/^ADDR=[^;]*/ADDR=$ADDR/" \
		-e "s/^MACHINE=[^;]*/MACHINE=$MACHINE/" \
		-e "s/^NETDEV=[^;]*/NETDEV=$NETDEV/" \
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

# We could be more intelligent and shutdown everything, but...
echo "Shutting down running services"
case "$HOSTTYPE" in
sgi)
	killall inetd snmpd syslogd named
	;;
*)
	if [ -f /var/athena/inetd.pid ]; then
		kill `cat /var/athena/inetd.pid` > /dev/null 2>&1
	fi
	if [ -f /etc/syslog.pid ]; then
		kill `cat /etc/syslog.pid` > /dev/null 2>&1
	fi
	if [ -f /var/athena/snmpd.pid ]; then
		kill `cat /var/athena/snmpd.pid` > /dev/null 2>&1
	fi
	if [ -f /etc/named.pid ]; then
		kill `cat /etc/named.pid` > /dev/null 2>&1
	fi
	;;
esac

# MINIROOT is currently only used for Irix 6.x.
if [ "$MINIROOT" = true ]; then
	# Set up a miniroot in the swap partition. We will boot into
	# it, and update-os will be run from there.

	echo "Suppressing network daemons for reboot"
	chkconfig -f suppress-network-daemons on

	sh /srvd/usr/athena/lib/update/setup-swap-boot "$method" "$NEWVERS"
	case "$?" in
	0)
		echo "Rebooting into swap to update OS files..."
		;;
	2)
		echo "Rebooting to clear swap..."
		echo "Athena Workstation ($HOSTTYPE) Version" \
                      "ClearSwap $method $NEWVERS `date`" \
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
else
	sh /srvd/usr/athena/lib/update/update-os

	case "$HOSTTYPE" in
	sgi)
		echo "Suppressing network daemons for reboot"
		chkconfig -f suppress-network-daemons on
		;;
	esac

	echo "Updating version for reboot"
	echo "Athena Workstation ($HOSTTYPE) Version Reboot" \
		"$method $NEWVERS `date`" >> "$CONFDIR/version"

	sync
fi

#!/bin/sh
# $Id: do-update.sh,v 1.48 2005-04-02 18:45:43 rbasch Exp $

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

# Let reactivate know that an update is running.
onexit()
{
	rm -rf /var/athena/update.running
}
trap onexit EXIT
touch /var/athena/update.running

# We get three arguments: the method by which the machine was rebooted.
# (possible values are Auto, Manual, and Remote), the current workstation
# version, and the new version.
method=$1
version=$2
newvers=$3

echo "Beginning update from $version to $newvers at `date`."
. /srvd/usr/athena/lib/update/update-environment

# Remove the version script state files.
rm -f "$CONFCHG" "$CONFVARS" "$AUXDEVS" "$OLDBINS" "$OLDLIBS" "$DEADFILES"
rm -f "$OSCONFCHG" "$PACKAGES" "$PATCHES" "$OLDPKGS" "$OLDPTCHS"
rm -f "$OSPRESERVE"
if [ -n "$PACKAGES" ]; then
      rm -f "$PACKAGES".*
fi
rm -f $MIT_CORE_PACKAGES $MIT_NONCORE_PACKAGES $MIT_OLD_PACKAGES

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
	cp /dev/null "$OSCONFCHG"
	cp /dev/null "$OSPRESERVE"
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
#	$PACKAGES	A list of OS packages to be de/installed
#	$OSCONFCHG	A list of config files to save/restore around
#			an update to the OS
#	$OSPRESERVE	A list of config files to save/restore around
#			the removal of OS packages
#	$CONFVARS	Can set variables to "true", including:
#		NEWUNIX		Update kernel
#		NEWBOOT		Boot blocks have changed
#		NEWDEVS		New pseudo-devices required
#		NEWOS		OS version has changed
#		TRACKOS		OS files relevant to local disk have changed
#		MINIROOT	some OS files require a miniroot update
#		OSCHANGES	Need to run /srvd/install/oschanges
#				(always set if any OS packages are installed)

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
		-e "s#^PUBLIC=[^;]*#PUBLIC=$PUBLIC#" \
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

		echo "The following variables are being added to" \
			"/etc/athena/rc.conf:"
		echo "	$vars"
		for i in $vars; do
			grep "^$i=" "/srvd$CONFDIR/rc.conf" \
				>> "$CONFDIR/rc.conf"
		done
	fi
fi

# Determine if there are any Athena packages to add or remove.  We do
# this here because the miniroot may not be able to run the pkg-order
# program used below.

# The logic here is to compare the package list given by the
# .order-version file (containing each package name and its version)
# in the new version's packages directory against the list of
# packages, with their version numbers, currently installed on the
# system.  Unique lines in the new version list are packages to be
# installed or updated.  To determine packages to be removed we
# compare only the package names from the two lists; packages named in
# only the currently installed list are slated for removal.

# Most Athena packages are installed in finish-update; if this is a
# miniroot update, finish-update will not run until after we reboot
# into the updated root.  However, we must make sure that any packages
# needed for finish-update to be able to do its work (e.g. AFS) are
# installed prior to that reboot, since previous versions of such
# software may not run any more.  These packages are marked as
# belonging to the "MITcore" category in pkginfo.  So we further
# separate the list of packages to be added into core and non-core
# lists; the former are installed by update-os, i.e. prior to
# rebooting into the updated root.

echo "Determining update actions ..."

old_list=/tmp/old_list$$
new_list=/tmp/new_list$$
old_verlist=/tmp/old_verlist$$
new_verlist=/tmp/new_verlist$$
add_list=/tmp/add_list$$
core_list=/tmp/core_list$$

pkgdir="/srvd/pkg/$newvers"
install_order="$pkgdir/.order-version"
if [ ! -s "$install_order" ]; then
	echo "Cannot find $install_order, aborting update." 1>&2
	exit 1
fi

rm -rf $old_list $new_list $old_verlist $new_verlist $add_list $core_list

# Get the list of currently installed Athena packages, with and without
# version numbers.
pkginfo -l -R "${UPDATE_ROOT:-/}" | awk '
	/PKGINST:/ { pkg = $2; }
	/VERSION:/ { if (pkg ~ /^MIT-/) print pkg, $2; pkg = ""; }' \
	| sort -u | tee $old_verlist | awk '{ print $1; }' > $old_list

# Sort the package list for the version we are updating to.
sort -u $install_order | tee $new_verlist | awk '{ print $1; }' > $new_list

# Get the new or updated packages in the new list, stripping off the
# version number.
comm -13 $old_verlist $new_verlist | awk '{ print $1; }' > $add_list
if [ -s $add_list ]; then
	# We have a list of packages to install.  We must split this list
	# into lists of core and non-core packages; the former must be
	# installed by update-os.  So we use pkginfo to select core
	# packages from the list of packages to be added.
	pkginfo -c MITcore -d $pkgdir `cat $add_list` 2>/dev/null \
		| awk '{ print $2; }' | sort -u > $core_list
	corepkgs=`cat $core_list`
	if [ -n "$corepkgs" ]; then
		# We have core packages to install.  Get them in
		# install order.
		$LIBDIR/pkg-order -d $pkgdir $corepkgs \
			> $MIT_CORE_PACKAGES || exit 1
	fi
	noncorepkgs=`comm -23 $add_list $core_list`
	if [ -n "$noncorepkgs" ]; then
		# We have noncore packages to install.  Get them in
		# install order.
		$LIBDIR/pkg-order -d $pkgdir $noncorepkgs \
			> $MIT_NONCORE_PACKAGES || exit 1
	fi
fi

# Get the list of packages present in the old list, but not the new
# list, i.e. the packages to be removed.
oldpkgs=`comm -23 $old_list $new_list`
if [ -n "$oldpkgs" ]; then
	# Get the order in which to remove the packages; the depend
	# file should be saved in the system pkg directory.  If
	# that fails, just remove them in the sorted order.
	$LIBDIR/pkg-order -r -d /var/sadm/pkg $oldpkgs \
		> $MIT_OLD_PACKAGES || echo "$oldpkgs" > $MIT_OLD_PACKAGES
fi

rm -f $old_list $new_list $old_verlist $new_verlist $add_list $core_list

if [ "$MINIROOT" = true ]; then
	# Set up a miniroot in the swap partition. We will boot into
	# it, and update-os will be run from there.

	case "$HOSTTYPE" in
	sgi)
		echo "Suppressing network daemons for reboot"
		chkconfig -f suppress-network-daemons on

		# Note the volume header must be updated before the miniroot
		# can boot (Irix only).
		if [ "$NEWBOOT" = true ]; then
			# Make sure the volume header has an up-to-date sash.
			echo "Updating sash volume directory entry..."
			dvhtool -v creat /install/lib/sash sash
		fi
		;;
	esac

	sh /install/miniroot/setup-swap-boot "$method" "$newvers"
	case "$?" in
	0)
		echo "Rebooting into swap to update OS files..."
		;;
	2)
		echo "Rebooting to clear swap..."
		echo "Athena Workstation ($HOSTTYPE) Version" \
                      "ClearSwap $method $newvers `date`" \
			>> "$CONFDIR/version"
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

sh /srvd/usr/athena/lib/update/update-os "$newvers"

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

#!/bin/sh
# $Id: do-update.sh,v 1.13 1997-04-03 06:24:07 ghudson Exp $

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

export CONFCHG CONFVARS AUXDEVS OLDBINS DEADFILES CONFDIR LIBDIR SERVERDIR PATH
export HOSTTYPE CPUTYPE

CONFCHG=/tmp/conf.list
CONFVARS=/tmp/update.conf
AUXDEVS=/tmp/driver.list
OLDBINS=/tmp/bins.list
DEADFILES=/tmp/dead.list
CONFDIR=/etc/athena
LIBDIR=/srvd/usr/athena/lib/update
SERVERDIR=/var/server
PATH=/bin:/etc:/usr/bin:/usr/ucb:/usr/bsd:/os/bin:/os/etc:/srvd/etc/athena:/srvd/bin/athena:/os/usr/bin:/srvd/usr/athena/etc:/os/usr/ucb:/os/usr/bsd:$LIBDIR
HOSTTYPE=`/srvd/bin/athena/machtype`
CPUTYPE=`/srvd/bin/athena/machtype -c`

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
#	$CONFVARS	Can set variables to "true", including:
#		NEWUNIX		Update kernel
#		NEWBOOT		Boot blocks have changed
#		NEWOS		OS version has changed
#		TRACKOS		OS files relevant to local disk have changed

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

echo "Backing up $CONFDIR/rc.conf to $CONFDIR/rc.conf.old."
rm -f "$CONFDIR/rc.conf.old"
cp -p "$CONFDIR/rc.conf" "$CONFDIR/rc.conf.old"

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
	killall inetd snmpd syslogd snmpd named
	;;
*)
	if [ -f /var/athena/inetd.pid ]; then
		kill `cat /var/athena/inetd.pid` > /dev/null 2>&1
	fi
	# 8.0 and prior machines still have /etc/athena/inetd.pid.
	if [ -f /etc/athena/inetd.pid ]; then
		kill `cat /etc/athena/inetd.pid` > /dev/null 2>&1
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

if [ -s "$OLDBINS" ]; then
	echo "Making copies of OS binaries we need"
	mkdir -p /tmp/bin
	bins="`cat $OLDBINS`"
	for i in $bins; do
		cp -p $i /tmp/bin/`basename $i`
	done
	PATH=/tmp/bin:$PATH; export PATH
fi

if [ -s "$DEADFILES" ]; then
	echo "Removing outdated files"
	dead="`cat $DEADFILES`"
	for i in $dead; do
		rm -rf $i
	done
fi

echo "Tracking changes"
if [ "$TRACKOS" = true ]; then
	case "$HOSTTYPE" in
	sgi)
		/install/install/track -v -F /install -T / -d \
			-W /install/install/lib
		;;
	sun4)
		# Sun ships multiple revisions of OS config files
		# with the same timestamp, so we must use -c.
		track -c -v -F /os -T / -d -W /srvd/usr/athena/lib \
			-s stats/os_rvd slists/os_rvd
		;;
	esac
fi
track -v -F /srvd -T / -d -W /srvd/usr/athena/lib
rm -f /var/athena/rc.conf.sync

if [ "$NEWOS" = true ]; then
	case "$HOSTTYPE" in
	sun4)
		echo "Copying new system files"
		cp -p /os/etc/driver_aliases /etc/driver_aliases
		cp -p /os/etc/name_to_major /etc/name_to_major
		;;
	esac
fi

if [ "$NEWUNIX" = true ] ; then
	echo "Updating kernel"
	case "$HOSTTYPE" in
	sun4)
		echo "Tracking new kernel"        
		track -c -v -F /os/kernel -T /kernel -d \
			-W /srvd/usr/athena/lib -s stats/kernel_rvd \
			slists/kernel_rvd
		echo "Tracking new usr kernel"        
		track -c -v -F /os/usr/kernel -T /usr/kernel -d \
			-W /srvd/usr/athena/lib -s stats/usr_kernel_rvd \
			slists/usr_kernel_rvd
		cp -p /srvd/kernel/drv/* /kernel/drv/
		cp -p /srvd/kernel/fs/* /kernel/fs/
		cp -p /srvd/kernel/strmod/* /kernel/strmod/
		;;
	sgi)
		/install/install/update
		;;
	esac
fi

if [ -s "$AUXDEVS" ]; then
	drvrs=`cat "$AUXDEVS"`
	for i in $drvrs; do
		/srvd/install/aux.devs/$i
	done
fi

if [ "$NEWBOOT" = true ]; then
	echo "Copying new bootstraps"

	case "$HOSTTYPE" in
	sun4)
		case "$VERSION" in
		7*|8.0*)
			# uname -i doesn't work with kernels prior to Solaris
			# 2.5.1.  `uname -m` works for now.
			platform=`uname -m`
			;;
		*)
			platform=`uname -i`
			;;
		esac
		/usr/sbin/installboot \
			"/usr/platform/$platform/lib/fs/ufs/bootblk" \
			/dev/rdsk/c0t3d0s0
		;;
	esac
fi

case "$HOSTTYPE" in
sgi)
	echo "Suppressing network daemons for reboot"
	chkconfig -f suppress-network-daemons on
	;;
esac

if [ "$AUTO" = true ]; then
	method=Auto
else
	method=Manual
fi

echo "Updating version for reboot"
echo "Athena Workstation ($HOSTTYPE) Version Reboot $method $NEWVERS `date`" \
	>> "$CONFDIR/version"

sync

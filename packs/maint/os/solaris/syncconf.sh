#!/bin/sh
# $Id: syncconf.sh,v 1.5 1999-12-06 04:25:48 ghudson Exp $

rcconf=/etc/athena/rc.conf
rcsync=/var/athena/rc.conf.sync
rcsyncout=$rcsync
handled=
rc2added=
all=
debug=
startup=
echo=echo
maybe=

# Usage: syncrc level {K/S} order scriptname boolvalue
# e.g. "syncrc 2 S 50 mail false" turns off the /etc/rc2.d/S50mail link
syncrc()
{
	uprefix=$2
	lprefix=`echo $uprefix | tr SK sk`
	if [ "$5" = false ]; then
		prefix="$lprefix"
	else
		prefix="$uprefix"
	fi
	if [ "$1$prefix" = 2S -a ! -h "/etc/rc2.d/S$3$4" ]; then
		mustreboot=true
	fi
	$maybe rm -f "/etc/rc$1.d/$lprefix$3$4" "/etc/rc$1.d/$uprefix$3$4"
	$maybe ln -s "../init.d/$4" "/etc/rc$1.d/$prefix$3$4"
}

remove()
{
	$maybe rm -f "$1"
}

move()
{
	$maybe mv -f "$1" "$2"
}

quiet_move()
{
	if [ -f "$1" ]; then
		move "$1" "$2"
	fi
}

put()
{
	if [ -n "$debug" ]; then
		$echo "echo $2 > $1"
	else
		echo "$2" > "$1"
	fi
}

append()
{
	if [ -n "$debug" ]; then
		$echo "echo $2 >> $1"
	else
		echo "$2" >> "$1"
	fi
}

handle()
{
	# Don't handle anything twice.
	case "$handled" in
	*" $1 "*)
		return
		;;
	esac
	handled="$handled $1 "

	case "$1" in
	NFSSRV)
		syncrc 0 K 66 nfs.server "$NFSSRV"
		syncrc 1 K 65 nfs.server "$NFSSRV"
		syncrc 2 K 60 nfs.server "$NFSSRV"
		syncrc 3 S 15 nfs.server "$NFSSRV"
		;;

	NFSCLIENT)
		syncrc 0 K 75 nfs.client "$NFSCLIENT"
		syncrc 1 K 80 nfs.client "$NFSCLIENT"
		syncrc 2 K 65 nfs.client "$NFSCLIENT"
		syncrc 2 S 73 nfs.client "$NFSCLIENT"
		;;

	HOSTADDR)
		oldhost=`cat /etc/nodename`
		oldaddr=`awk '{ a = $1; } END { print a; }' /etc/inet/hosts`
		wasdhcp=false
		if [ -f /etc/dhcp.$NETDEV ]; then
			wasdhcp=true
		fi

		move /etc/nodename /etc/nodename.saved
		move "/etc/hostname.$NETDEV" "/etc/hostname.$NETDEV.saved"
		move /etc/defaultrouter /etc/defaultrouter.saved
		move /etc/inet/hosts /etc/inet/hosts.saved
		move /etc/inet/netmasks /etc/inet/netmasks.saved

		put	/etc/inet/hosts "#"
		append	/etc/inet/hosts "# Internet host table"
		append	/etc/inet/hosts "#"
		append	/etc/inet/hosts "127.0.0.1  localhost loghost"

		if [ "$ADDR" = dhcp ]; then
			$maybe touch /etc/nodename /etc/hostname.$NETDEV
			$maybe touch /etc/defaultrouter /etc/netmasks
			put "/etc/dhcp.$NETDEV" "primary"
		else
			remove "/etc/dhcp.$NETDEV"
			set -- `/etc/athena/netparams "$ADDR"`
			netmask=$1
			gateway=$4

			# Get the first component of the hostname for the hosts
			# file.
			first=`expr "$HOST" : '\([^.]*\)\.'`

			put	/etc/nodename "$HOST"
			put	"/etc/hostname.$NETDEV" "$HOST"
			put	/etc/defaultrouter "$gateway"
			append	/etc/inet/hosts "$ADDR  $HOST $first"
			put	/etc/inet/netmasks "# Netmask for this host"
			append	/etc/inet/netmasks "$ADDR	$netmask"
		fi

		# Decrease the likelihood that the machine reboots
		# uncleanly before the new files are written out to
		# disk.  A more robust algorithm is in the works.
		sync

		# Hostname configuration happens prior to rc2 scripts on
		# Solaris.
		if [ "$ADDR" = dhcp -a "$wasdhcp" = false ]; then
			mustreboot=true
		fi
		if [ "$ADDR" != dhcp ]; then
			if [ "$wasdhcp" = true -o "$HOST" != "$oldhost" -o \
			     "$ADDR" != "$oldaddr" ]; then
				mustreboot=true
			fi
		fi
		;;

	MAILRELAY)
		case $MAILRELAY in
		none)
			remove /etc/athena/sendmail.conf
			;;
		default)
			case $ADDR,$HOST in
			dhcp,*)
				remove /etc/athena/sendmail.conf
				;;
			*,*.MIT.EDU|*,*.mit.edu)
				put /etc/athena/sendmail.conf \
					"relay ATHENA.MIT.EDU"
				;;
			*,*)
				remove /etc/athena/sendmail.conf
				;;
			esac
			;;
		*)
			put /etc/athena/sendmail.conf "relay $MAILRELAY"
			;;
		esac
		;;

	*)
		$echo "syncconf: unknown variable $1"
		;;
	esac
}

while getopts anq opt; do
	case "$opt" in
	a)
		all=1
		;;
	n)
		debug=1
		rcsyncout=/tmp/rc.conf.sync
		maybe=$echo
		;;
	q)
		echo=:
		;;
	\?)
		echo "Usage: syncconf [-anq]"
		exit 1
		;;
	esac
done
shift `expr $OPTIND - 1`
if [ "$#" -ne 0 ]; then
	echo "Usage: syncconf [-anq]"
	exit 1
fi

$echo "Synchronizing configuration... \c"

. "$rcconf"

if [ -z "$all" -a -f "$rcsync" ]; then
	. "$rcsync"
else
	changes="NFSSRV NFSCLIENT HOSTADDR MAILRELAY"
fi

if [ -z "$changes" ]; then
	$echo "No changes to synchronize."
	exit
fi

for i in $changes; do
	$echo "$i \c"
	if [ -n "$debug" ]; then
		$echo ""
	fi
	handle "$i"
done

for i in $dependencies; do
	$echo "($i) \c"
	if [ -n "$debug" ]; then
		$echo ""
	fi
	handle "$i"
done

$echo ""

cat > $rcsyncout << EOF
# This file was generated by /etc/athena/syncconf; do not edit.
if [ \$NFSSRV != $NFSSRV ]; then changes="\$changes NFSSRV"; fi
if [ \$NFSCLIENT != $NFSCLIENT ]; then changes="\$changes NFSCLIENT"; fi
if [ \$HOST != $HOST ]; then changes="\$changes HOSTADDR MAILRELAY"; fi
if [ \$ADDR != $ADDR ]; then changes="\$changes HOSTADDR MAILRELAY"; fi
if [ \$MAILRELAY != $MAILRELAY ]; then changes="\$changes MAILRELAY"; fi
EOF

if [ -n "$mustreboot" ]; then
	# Exit with status 1 to indicate need to reboot if run at startup.
	exit 1
fi

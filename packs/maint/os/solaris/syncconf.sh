#!/bin/sh
# $Id: syncconf.sh,v 1.10 2001-04-13 20:07:52 ghudson Exp $

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

update()
{
	$maybe ln -f "$1" "$1.saved"
	$maybe /bin/athena/syncupdate "$1.new" "$1"
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

		remove /etc/nodename.new
		remove /etc/hotname.$NETDEV.new
		remove /etc/defaultrouter.new
		remove /etc/inet/hosts.new
		remove /etc/inet/netmasks.new

		put	/etc/inet/hosts.new "#"
		append	/etc/inet/hosts.new "# Internet host table"
		append	/etc/inet/hosts.new "#"
		append	/etc/inet/hosts.new "127.0.0.1  localhost loghost"

		if [ "$ADDR" = dhcp ]; then
			$maybe touch /etc/nodename.new
			$maybe touch /etc/hostname.$NETDEV.new
			$maybe touch /etc/defaultrouter.new
			$maybe touch /etc/netmasks.new
			put "/etc/dhcp.$NETDEV" "primary"
		else
			remove "/etc/dhcp.$NETDEV"
			set -- `/etc/athena/netparams "$ADDR"`
			netmask=$1
			net=$2
			gateway=$4

			# Get the first component of the hostname for the hosts
			# file.
			first=`expr "$HOST" : '\([^.]*\)\.'`

			put    /etc/nodename.new "$HOST"
			put    "/etc/hostname.$NETDEV.new" "$HOST"
			put    /etc/defaultrouter.new "$gateway"
			append /etc/inet/hosts.new "$ADDR  $HOST $first"
			put    /etc/inet/netmasks.new "# Netmask for this net"
			append /etc/inet/netmasks.new "$net	$netmask"
		fi

		update /etc/nodename
		update /etc/hostname.$NETDEV
		update /etc/defaultrouter
		update /etc/inet/hosts
		update /etc/inet/netmasks

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

	SAVECORE)
		mkdir -p -m 0700 "/var/crash/$HOST"
		dumpadm -s "/var/crash/$HOST"
		case $SAVECORE in
		false)
			dumpadm -n
			;;
		*)
			dumpadm -y
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
	changes="NFSSRV NFSCLIENT HOSTADDR MAILRELAY SAVECORE"
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
if [ \$HOST != $HOST ]; then
  changes="\$changes HOSTADDR MAILRELAY SAVECORE"
fi
if [ \$ADDR != $ADDR ]; then changes="\$changes HOSTADDR MAILRELAY"; fi
if [ \$MAILRELAY != $MAILRELAY ]; then changes="\$changes MAILRELAY"; fi
if [ \$SAVECORE != $SAVECORE ]; then changes="\$changes SAVECORE"; fi
EOF

if [ -n "$mustreboot" ]; then
	# Exit with status 1 to indicate need to reboot if run at startup.
	exit 1
fi

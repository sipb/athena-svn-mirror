#!/bin/sh
# $Id: syncconf.sh,v 1.8 2000-08-01 22:40:30 rbasch Exp $

config=/etc/config
setconfig="/sbin/chkconfig -f"
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

configbool()
{
	$maybe $setconfig "$1" "$2"
}

configopt()
{
	if [ -n "$debug" ]; then
		$echo "echo $2 > $config/$1"
	else
		echo "$2" > "$config/$1"
	fi
}

syncvar()
{
	case "$2" in
	true|on)
		configbool "$1" on
		;;
	false|off)
		configbool "$1" off
		;;
	*)
		configopt "$1" "$2"
		;;
	esac
}

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
		rc2added=1
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
	TIMECLIENT)
		# gettime and AFS time synchronization
		syncvar timeclient "$TIMECLIENT"
		dependencies="$dependencies AFSCLIENT"
		;;

	TIMEHUB)
		syncvar timeclient.options "$TIMEHUB"
		;;

	TIMESRV)
		syncvar timed "$TIMESRV"
		;;

	AFSCLIENT)
		if [ "$AFSCLIENT" != false ]; then

			syncvar afsml false
			syncvar afsclient true

			afsclientnum=4
			if [ "$AFSCLIENT" != "true" ]; then
				afsclientnum="$AFSCLIENT"
			fi

			options="-stat 2000 -dcache 800 -volumes 70"
			options="$options -daemons $afsclientnum"
			if [ "$TIMECLIENT" = false ]; then
				options="$options -nosettime"
			fi

			syncvar afsd.options "$options"
		else
			syncvar afsml false
			syncvar afsclient false
		fi
		;;

	NFS)
		if [ "$NFSSRV" != false -o "$NFSCLIENT" != false ]; then
			syncvar nfs true

		        case "$NFSCLIENT" in
			true|false)
				remove $config/biod.options
				;;
			*)
				syncvar biod.options "$NFSCLIENT"
				;;
			esac

			case "$NFSSRV" in
	                true)
				remove $config/nfsd.options
				syncvar lockd true
				;;
			false)
				syncvar nfsd.options 0
				syncvar lockd false
				;;
			*)
				syncvar nfsd.options "$NFSSRV"
				syncvar lockd true
				;;
			esac
		else
			syncvar nfs false
		fi
		;;

	SENDMAIL)
		syncvar sendmail "$SENDMAIL"
		;;

	SAVECORE)
		if [ "$SAVECORE" != false ]; then
			case "$SAVECORE" in
			true)
				minfree=30000000
				;;
			*)
				minfree="$SAVECORE"
				;;
			esac
			put /var/adm/crash/minfree $minfree
		fi
		syncrc 2 S 48 savecore "$SAVECORE"
		;;

	ACCOUNT)
		syncvar acct "$ACCOUNT"
		;;

	QUOTAS)
		syncvar quotacheck "$QUOTAS"
		syncvar quotas "$QUOTAS"
		;;

	HOSTADDR)
		remove /etc/sys_id.new
		remove /etc/hosts.new
		remove $config/static-route.options.new
		remove $config/ifconfig-1.options.new

		set -- `/etc/athena/netparams "$ADDR"`
		netmask=$1
		broadcast=$3
		gateway=$4

		# Get the first component of the hostname for the hosts
		# file.
		first=`expr "$HOST" : '\([^.]*\)\.'`

		put    /etc/sys_id.new $HOST

		put    $config/static-route.options.new "#"
		append $config/static-route.options.new \
"# See /os/etc/config/static-route.options for information on this file."
		append $config/static-route.options.new "#"
		append $config/static-route.options.new \
			"\$ROUTE \$QUIET add net default $gateway"

		put    $config/ifconfig-1.options.new \
			"netmask $netmask broadcast $broadcast"

		put    /etc/hosts.new "#"
		append /etc/hosts.new "# Internet host table"
		append /etc/hosts.new "#"
		append /etc/hosts.new "127.0.0.1  localhost"
		append /etc/hosts.new "$ADDR  $HOST $first"

		update /etc/sys_id
		update /etc/hosts
		update $config/static-route.options
		update $config/ifconfig-1.options

		/sbin/nvram netaddr "$ADDR"
		;;

	MAILRELAY)
		case $MAILRELAY in
		none)
			remove /etc/athena/sendmail.conf
			;;
		default)
			case $HOST in
			*.MIT.EDU|*.mit.edu)
				put /etc/athena/sendmail.conf \
					"relay ATHENA.MIT.EDU"
				;;
			*)
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
	changes="TIMECLIENT TIMEHUB TIMESRV AFSCLIENT NFS SENDMAIL"
	changes="$changes SAVECORE ACCOUNT QUOTAS HOSTADDR MAILRELAY"
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
if [ \$TIMECLIENT != $TIMECLIENT ]; then changes="\$changes TIMECLIENT"; fi
if [ \$TIMEHUB != $TIMEHUB ]; then changes="\$changes TIMEHUB"; fi
if [ \$TIMESRV != $TIMESRV ]; then changes="\$changes TIMESRV"; fi
if [ \$AFSCLIENT != $AFSCLIENT ]; then changes="\$changes AFSCLIENT"; fi
if [ \$NFSSRV != $NFSSRV ]; then changes="\$changes NFS"; fi
if [ \$NFSCLIENT != $NFSCLIENT ]; then changes="\$changes NFS"; fi
if [ \$SENDMAIL != $SENDMAIL ]; then changes="\$changes SENDMAIL"; fi
if [ \$SAVECORE != $SAVECORE ]; then changes="\$changes SAVECORE"; fi
if [ \$ACCOUNT != $ACCOUNT ]; then changes="\$changes ACCOUNT"; fi
if [ \$QUOTAS != $QUOTAS ]; then changes="\$changes QUOTAS"; fi
if [ \$HOST != $HOST ]; then changes="\$changes HOSTADDR MAILRELAY"; fi
if [ \$ADDR != $ADDR ]; then changes="\$changes HOSTADDR"; fi
if [ \$MAILRELAY != $MAILRELAY ]; then changes="\$changes MAILRELAY"; fi
EOF

if [ -n "$rc2added" ]; then
	# Exit with status 1 to indicate need to reboot if run at startup.
	exit 1
fi

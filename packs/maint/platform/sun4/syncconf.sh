#!/bin/sh
# $Id: syncconf.sh,v 1.1 1997-02-15 20:02:47 ghudson Exp $

rcconf=/etc/athena/rc.conf
rcsync=/etc/athena/.rc.conf.sync
rcsyncout=$rcsync
handled=
rc2added=
all=
debug=
startup=
echo=echo
maybe=

# Usage: syncrc2 scriptname order value
# e.g. "syncrc2 mail 50 false" turns off the /etc/rc2.d/S50mail link.
syncrc2()
{
	if [ "$3" = false ]; then
		prefix=s
	else
		prefix=S
	fi
	if [ "$3" != false -a ! -h "/etc/rc2.d/S$2$1" ]; then
		rc2added=1
	fi
	$maybe rm -f "/etc/rc2.d/s$2$1" "/etc/rc2.d/S$2$1"
	$maybe ln -s "../init.d/$2" "/etc/rc2.d/$prefix$2$1"
}

# Usage: syncrc scriptname order value
# e.g. "syncrc0 mail 20 true" turns on the /etc/rc2.d/K20mail link.
syncrc0()
{
	if [ "$3" = false ]; then
		prefix=k
	else
		prefix=K
	fi
	$maybe rm -f "/etc/rc0.d/k$2$1" "/etc/rc0.d/K$2$1"
	$maybe ln -s "../init.d/$2" "/etc/rc0.d/$prefix$2$1"
}

remove()
{
	$maybe rm -f "$1"
}

move()
{
	$maybe mv "$1" "$2"
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
		syncrc2 nfs.server 99 "$NFSSRV"
		syncrc0 nfs.server 66 "$NFSSRV"
		;;

	NFSCLIENT)
		syncrc2 nfs.client 73 "$NFSCLIENT"
		syncrc0 nfs.client 75 "$NFSCLIENT"
		;;

	HOSTADDR)
		move /etc/nodename /etc/nodename.saved
		move /etc/hostname.le0 /etc/hostname.le0.saved
		move /etc/defaultrouter /etc/defaultrouter.saved
		move /etc/inet/hosts /etc/inet/hosts.saved

		net=`echo $ADDR | awk -F. '{ print $1 "." $2 }'`
		gateway=$net.0.1
		broadcast=$net.255.255

		put	/etc/nodename "$HOST"
		put	/etc/hostname.le0 "$HOST"
		put	/etc/defaultrouter "$gateway"
		put	/etc/inet/hosts "#"
		append	/etc/inet/hosts "# Internet host table"
		append	/etc/inet/hosts "#"
		append	/etc/inet/hosts "127.0.0.1  localhost loghost"
		append	/etc/inet/hosts "$ADDR  $HOST.MIT.EDU $HOST"
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
	changes="NFSSRV NFSCLIENT HOSTADDR"
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
if [ \$NFSSRV != $NFSSRV ]; then changes="\$changes NFSSRV"; fi
if [ \$NFSCLIENT != $NFSCLIENT ]; then changes="\$changes NFSCLIENT"; fi
if [ \$HOST != $HOST ]; then changes="\$changes HOSTADDR"; fi
if [ \$ADDR != $ADDR ]; then changes="\$changes HOSTADDR"; fi
EOF

if [ -n "$rc2added" ]; then
	# Exit with status 1 to indicate need to reboot if run at startup.
	exit 1
fi

#!/sbin/sh

if [ -f $CONFVARS ]; then
	. $CONFVARS
fi

if [ -s "$DEADFILES" ]; then
	echo "Removing outdated files"
	dead="`cat $DEADFILES`"
	for i in $dead; do
		rm -rf $i
	done
fi

# This block of code is applicable only to Irix 6.x.
if [ -s $LOCALPACKAGES -o -s $LINKPACKAGES ]; then
	echo "Running inst to make operating system changes..."

	opts="-a -Vinstmode:normal -Vstartup_script:ignore -N"
	opts="$opts -r $UPDATE_ROOT/"

	if [ -s $LOCALPACKAGES ]; then
		echo "Updating local operating system packages"
		cat /install/selections/header $LOCALPACKAGES > \
		    /tmp/selections.local

		inst $opts -F /tmp/selections.local
	fi

	if [ -s $LINKPACKAGES ]; then
		echo "Updating linked operating system packages"
		cat /install/selections/header $LINKPACKAGES > \
		    /tmp/selections.link

		inst $opts -T/os -F /tmp/selections.link -Vskip_rqs:true
	fi

	sh /srvd/install/oschanges --root $UPDATE_ROOT/
fi

echo "Tracking changes"

# Irix 5.3 only.
if [ "$TRACKOS" = true ]; then
	/install/install/track -v -F /install -T / -d \
		-W /install/install/lib
fi

# Track from the srvd.
track -v -F /srvd -T $UPDATE_ROOT/ -d -W /srvd/usr/athena/lib
rm -f $UPDATE_ROOT/var/athena/rc.conf.sync

# Irix 5.3 only.
if [ "$NEWUNIX" = true ] ; then
	echo "Updating kernel"
	/install/install/update
fi

#!/sbin/sh

if [ -f $CONFVARS ]; then
	. $CONFVARS
fi

if [ -s "$DEADFILES" ]; then
	echo "Removing outdated files"
	dead="`cat $DEADFILES`"
	for i in $dead; do
		rm -rf $UPDATE_ROOT/$i
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

	# Restore any new config file that inst has replaced.
	# If inst has copied the file to a .O version, clean it up.
	# For a public workstation, remove any version file left by
	# inst (both .N and .O).
	if [ -s "$CONFCHG" ]; then
		for i in `cat $CONFCHG`; do
			if [ -f /srvd$i ]; then
				rm -rf ${UPDATE_ROOT}$i
				cp -p /srvd$i ${UPDATE_ROOT}$i
				if cmp -s ${UPDATE_ROOT}$i ${UPDATE_ROOT}$i.O
				then
					rm -f ${UPDATE_ROOT}$i.O
				fi
			fi
			if [ "$PUBLIC" = "true" ]; then
				rm -rf ${UPDATE_ROOT}$i.[NO]
			fi
		done
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

if [ "$NEWUNIX" = true -o "$NEWOS" = true ] ; then
	echo "Building kernel"
	/srvd/install/buildkernel --root $UPDATE_ROOT/ \
	    --disk `devnm $UPDATE_ROOT/ | \
			awk -F/ '{ print substr($4,4,1), substr($4,6,1) }'`
fi

#!/sbin/sh

PATH=/srvd/install:$PATH
export PATH

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

seldir=/install/selections
dists=`cat ${seldir}/dist_order 2>/dev/null`

# See if we need to run inst to update OS packages.
runinst=0
for dist in $dists ; do
	if [ -s $LOCALPACKAGES.$dist -o -s $LINKPACKAGES.$dist ]; then
		runinst=1
		break
	fi
done

if [ "$runinst" -ne 0 ]; then
	echo "Running inst to make operating system changes..."

	opts="-a -Vinstmode:normal -Vstartup_script:ignore -N"
	opts="$opts -Vdelay_idb_read:on -Voverlay_mode:silent"
	opts="$opts -r $UPDATE_ROOT/"

	# Install local packages.
	for dist in $dists ; do
		if [ -s $LOCALPACKAGES.$dist ]; then
			echo "Updating local $dist packages"
			cat $seldir/header.$dist $LOCALPACKAGES.$dist > \
				/tmp/selections.local

			inst $opts -F /tmp/selections.local
		fi
	done

	# Skip having inst rebuild the file type database by moving
	# the Makefile out of the way to install symlinks.  This is a
	# big win during a full install, when the make would normally
	# be invoked many times.  We'll restore the Makefile and do
	# one make after the symlink mode passes.
	if [ "$DELAYFTMAKE" = true ]; then
		ftmakefile=${UPDATE_ROOT}/usr/lib/filetype/Makefile
		mv ${ftmakefile} ${ftmakefile}.hold
	fi

	# Install symlink packages.
	for dist in $dists ; do
		if [ -s $LINKPACKAGES.$dist ]; then
			echo "Updating linked $dist packages"
			cat $seldir/header.$dist $LINKPACKAGES.$dist > \
				/tmp/selections.link

			inst $opts -T/os -F /tmp/selections.link \
				-Vskip_rqs:true
		fi
	done

	# Now put the file type database Makefile back, and do the make.
	if [ "$DELAYFTMAKE" = true ]; then
		mv ${ftmakefile}.hold ${ftmakefile}
		echo "Doing 'make' in /usr/lib/filetype..."
		chroot ${UPDATE_ROOT} /bin/sh -c \
		    "cd /usr/lib/filetype; make > /dev/null"
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

	# An update from the miniroot can leave the wrong disklinks
	# in /dev (e.g., because there is no swap partition).  Fix them.
	if [ "$MINIROOT" = true ]; then
		sh /srvd/install/tweakdev --root $UPDATE_ROOT/
	fi

	sh /srvd/install/oschanges --root $UPDATE_ROOT/
fi

# See if we need to update the documentation trees.
if [ "$UPDATEDOC" = true ]; then
	sh /srvd/install/docinst --root $UPDATE_ROOT/
fi

echo "Tracking changes"

# Track from the srvd.
track -v -F /srvd -T $UPDATE_ROOT/ -d -W /srvd/usr/athena/lib
rm -f $UPDATE_ROOT/var/athena/rc.conf.sync

if [ "$NEWUNIX" = true -o "$NEWOS" = true ] ; then
	echo "Building kernel"
	/srvd/install/buildkernel --root $UPDATE_ROOT/ \
	    --disk `devnm $UPDATE_ROOT/ | \
			awk -F/ '{ print substr($4,4,1), substr($4,6,1) }'`
fi
echo "Finished the first stage of the update at `date`."

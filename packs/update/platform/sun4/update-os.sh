#!/sbin/sh

if [ -f $CONFVARS ]; then
	. $CONFVARS
fi

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
	# Sun ships multiple revisions of OS config files
	# with the same timestamp, so we must use -c.
	track -c -v -F /os -T / -d -W /srvd/usr/athena/lib \
		-s stats/os_rvd slists/os_rvd

	# Bring this architecture's /platform directory local.
	rm -rf "/platform/$platform"
	cp -rp "/os/platform/$platform" "/platform/$platform"
fi

track -v -F /srvd -T / -d -W /srvd/usr/athena/lib
rm -f /var/athena/rc.conf.sync

if [ "$NEWOS" = true ]; then
	echo "Copying new system files"
	cp -p /os/etc/driver_aliases /etc/driver_aliases
	cp -p /os/etc/name_to_major /etc/name_to_major
fi

if [ "$NEWUNIX" = true ] ; then
	echo "Updating kernel"
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
fi

if [ "$NEWBOOT" = true ]; then
	echo "Copying new bootstraps"

	/usr/sbin/installboot \
		"/usr/platform/$platform/lib/fs/ufs/bootblk" \
		/dev/rdsk/c0t3d0s0
fi

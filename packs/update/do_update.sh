#!/bin/sh
#
# $Id: do_update.sh,v 1.3 1996-05-15 20:33:47 cfields Exp $
#

ROOT=${ROOT-}; export ROOT

CONFCHG=/tmp/conf.list; export CONFCHG
CONFVARS=/tmp/update.conf; export CONFVARS
CONFDIR="${CONFDIR-/etc/athena}"; export CONFDIR

LIBDIR=${LIBDIR-/srvd/usr/athena/lib/update};  export LIBDIR
PATH=/srvd/bin:/srvd/etc:/srvd/etc/athena:/srvd/bin/athena:/srvd/usr/bin:/srvd/usr/etc:/srvd/usr/athena/etc:/srvd/usr/ucb:/srvd/usr/new:$LIBDIR:/bin:/etc:/usr/bin:/usr/etc:/usr/ucb ; export PATH

# bsd or sysv style echo
TESTECHO=`echo -n`
case "$TESTECHO" in
   -n) N=''; C='\c';;
   *)  N='-n'; C='';;
esac

if [ ! -f ${ROOT}/${CONFDIR}/rc.conf -a -f ${ROOT}/etc/rc.conf ]; then
	CONFDIR=/etc; export CONFDIR;
fi

echo "Starting update..."
SITE=var; export SITE
SERVERDIR=${SITE}/server; export SERVERDIR
CP_P=-p; export CP_P
MTYPE=`/srvd/bin/athena/machtype -c` ; export MTYPE
mpublic="	/etc/crontab"

case $MTYPE in 
KN01)
	MACH=DS3100; export MACH
	CP_P=; export CP_P
	mpublic="	/etc/crontab \
			/etc/inetd.conf \
			/etc/elcsd.conf"
	;;
KN02)
	MACH=DS5000; export MACH
	CP_P=; export CP_P
	mpublic="	/etc/crontab \
			/etc/inetd.conf \
			/etc/elcsd.conf"
	;;
KN02ba)
	MACH=DS5000_100; export MACH
	CP_P=; export CP_P
	mpublic="	/etc/crontab \
			/etc/inetd.conf \
			/etc/elcsd.conf"
	;;
KN02ca)
	MACH=DSMAXINE; export MACH
	CP_P=; export CP_P
	mpublic="	/etc/crontab \
			/etc/inetd.conf \
			/etc/elcsd.conf"
	;;
KN210)
	MACH=DS5400; export MACH
	CP_P=; export CP_P
	mpublic="	/etc/crontab \
			/etc/inetd.conf \
			/etc/elcsd.conf"
	;;
POWER*)
	MACH=RSAIX; export MACH
	mpublic="	/etc/resolv.conf \
			/etc/inetd.conf \
			/etc/syslog.conf"
	;;
SPARC/IP*)
	MACH=SUN4c; export MACH
	mpublic="	/etc/resolv.conf
			/etc/syslog.conf
			/etc/inet/inetd.conf
			/etc/athena/inetd.conf
			/etc/minor_perm
			/etc/system"
	;;
# SPARC/Classic, 5, 10, 20
SPARC/*)
	MACH=SUN4m; export MACH
	mpublic="	/etc/resolv.conf
			/etc/syslog.conf
			/etc/inet/inetd.conf
			/etc/athena/inetd.conf
			/etc/minor_perm
			/etc/system"
	;;
IP22)
	MACH=INDY; export MACH
	mpublic="	/etc/resolv.conf \
			/etc/syslog.conf \
			/etc/inetd.conf \
			/etc/athena/inetd.conf \
			/var/spool/cron/crontabs/root"
	;;
*)
	echo "Unknown machine type...exiting"
	exit 1
	;;
esac

# If we are a public workstation, let us be paranoid and update everything.
public_files="	/.cshrc \
		/.login \
		/.profile \
		/.xsession \
		/etc/athena/attach.conf \
		/etc/ftpusers \
		/etc/syslog.conf \
		/etc/athena/newsyslog.conf \
		/etc/shells \
		${mpublic}"

if [ "${AFSCLIENT}" = "true" ]; then
public_files="	${public_files} \
		/usr/vice/etc/cacheinfo \
		/usr/vice/etc/SuidCells \
		/usr/vice/etc/ThisCell"
fi

if [ ! -d ${ROOT}/.deleted ]; then
	echo $N "Making tempdir...$C"
	mkdir ${ROOT}/.deleted
fi

# As nonstd will remove /etc/afsd (moved to /etc/athena)
if [ -f ${ROOT}/etc/afsd -a ! -f ${ROOT}/.deleted/etc.afsd ]; then
	ln ${ROOT}/etc/afsd ${ROOT}/.deleted/etc.afsd
fi

case $MTYPE in
POWER*)
	;;
SUN*|SPARC*)
	;;
IP22)
	;;
*)
	/srvd/usr/ucb/reset
	/bin/sh $LIBDIR/nonstd
	if [ $? != 0 ]; then exit 1; fi
	;;
esac

echo $N "Athena Workstation ($MACH) Version Update $C" >> ${CONFDIR}/version
date >> ${ROOT}${CONFDIR}/version

echo ""
echo "Backing up rc.conf to rc.conf.old"
rm -f ${ROOT}${CONFDIR}/rc.conf.old
cp ${CP_P} ${ROOT}${CONFDIR}/rc.conf ${ROOT}${CONFDIR}/rc.conf.old

if [ "${VERSION}" != "${NEWVERS}" ]; then
	echo "Version-specific updating.."
	cp /dev/null ${CONFCHG}
	cp /dev/null ${CONFVARS}
	upvers  $VERSION $NEWVERS $LIBDIR
fi

# First let us check for configuration changes
# The version-specific scripts may have set something.
# If it is a public workstation, on the other hand, let us update everything

if [ -f ${ROOT}/etc/athena/rc.conf ]; then
	CONFDIR=/etc/athena; export CONFDIR;
fi

. ${ROOT}${CONFDIR}/rc.conf
if [ -f ${CONFVARS} ]; then
	. ${CONFVARS}
fi

if [ "${PUBLIC}" = "true" ]; then
	NEWUNIX=true
	NEWBOOT=true
	case "${MTYPE}" in
	SPARC*)
		;;
	IP22)
		;;
	*)
		NEWTTYS=true
		;;
	esac
	FULLCOPY=true
	NEWMAILCF=true
	for i in $public_files; do echo "$i" >> ${CONFCHG}; done
	rm -f	${ROOT}/.hushlogin \
		${ROOT}/hesiod.conf \
		${ROOT}/etc/*.local ${ROOT}/etc/athena/*.local \
		${ROOT}/${SITE}/usr/lib/*.local \
		${ROOT}/usr/vice/etc/CellServDB \
		${ROOT}/usr/vice/etc/CellServDB.public
	cp /dev/null ${ROOT}/etc/named.local
fi
for i in $public_files; do
	if [ ! -f ${ROOT}/$i ]; then echo "$i" >> ${CONFCHG}; fi
done

case "${MTYPE}" in
SPARC*) echo "Updating root's crontab (old crontab is in /tmp/crontab.old)"
	cp /var/spool/cron/crontabs/root /tmp/crontab.old
	cp -p /srvd/etc/crontab.root.add  /var/spool/cron/crontabs/root
	;;
POWER*)	echo "Updating root's crontab (old crontab is in /tmp/crontab.old)"
	crontab -l > /tmp/crontab.old
	(sed -e '/Athena additions - BEGIN/,/Athena additions - END/d' \
		/tmp/crontab.old; \
		echo "# Athena additions - BEGIN"; \
		cat $LIBDIR/crontab.root.add; \
		echo "# Athena additions - END") \
	    | crontab
	;;
# On SGI, crontab is treated as a normal config file, because it
# can't easily be edited. I'm just using this case statement because
# it's here. All platforms should probably do their services by
# editing them, and the SPARC code above is doing a disservice.
IP22)
	echo "Updating /etc/services (old services is in /tmp/services.old)"
	cp /etc/services /tmp/services.old
	(sed -e '/Athena additions - BEGIN/,/Athena additions - END/d' \
		/tmp/services.old; \
		echo "# Athena additions - BEGIN"; \
		cat $LIBDIR/services.add; \
		echo "# Athena additions - END") > /etc/services
	;;
esac

if [ "${AFSCLIENT}" = "true" ]; then
	# Update CellServDB
	echo $N "Updating CellServDB.public...$C"
	if [ -s /afs/athena.mit.edu/service/CellServDB ]; then
		cp ${CP_P} /afs/athena.mit.edu/service/CellServDB \
			${ROOT}/usr/vice/etc/CellServDB.public
	else
		cp ${CP_P} /srvd/${SITE}/usr/vice/etc/CellServDB \
			${ROOT}/usr/vice/etc/CellServDB.public
	fi
	if [ ! -f ${ROOT}/usr/vice/etc/CellServDB ]; then
		rm -f ${ROOT}/usr/vice/etc/CellServDB
		ln -s CellServDB.public ${ROOT}/usr/vice/etc/CellServDB
	fi

	echo $N "aklog...$C"
	if [ -s /afs/athena.mit.edu/service/aklog ]; then
		cp ${CP_P} /afs/athena.mit.edu/service/aklog \
			${ROOT}/bin/athena/aklog
	else
		cp ${CP_P} /srvd/bin/athena/aklog \
			${ROOT}/bin/athena/aklog
	fi

	echo $N "Fixing permissions on /usr/vice/cache...$C"
	chmod 700 ${ROOT}/usr/vice/cache/
	echo "done."
fi

# Here is the list of configuration files that have changed in this release.
if [ "${NEWTTYS}" = "true" ]; then
	case $MTYPE in
	POWER*)
		;;
	*)	
		echo "/etc/ttys" >> ${CONFCHG}
		;;
	esac
	echo "/etc/athena/login/config" >> ${CONFCHG}
fi

if [ -s ${CONFCHG} ]; then
	conf="`cat ${CONFCHG} | sort -u`"
	if [ "${PUBLIC}" != "true" ]; then
	   echo "The following configuration files have changed and will be"
	   echo "replaced.  The old versions will be renamed to the same"
	   echo "name, but with a .old extension.  For example,"
	   echo "/etc/inetd.conf would be renamed to /etc/inetd.conf.old"
	   echo "and a new version would take its place."
	   echo ""
	   for i in $conf; do if [ -f $i ]; then echo "	$i"; fi; done
	   echo ""
	   echo $N "Press RETURN to continue --> $C"
	   read foo
	   echo ""
	   for i in $conf; do
	      if [ -f ${ROOT}/$i ]; then mv -f ${ROOT}/$i ${ROOT}/$i.old; fi
	   done
	fi
	for i in $conf; do rm -rf ${ROOT}/$i; cp ${CP_P} /srvd/$i ${ROOT}/$i; done
fi

# Verify /etc/athena/rc.conf has all the necessary variables
echo $N "Checking " ${CONFDIR}/"rc.conf...$C"
if [ "${PUBLIC}" = "true" ]; then
	sed -n	-e "s/^HOST=[^;]*/HOST=${HOST}/" \
		-e "s/^ADDR=[^;]*/ADDR=${ADDR}/" \
		-e "s/^MACHINE=[^;]*/MACHINE=${MACHINE}/" \
		-e "s/^NETDEV=[^;]*/NETDEV=${NETDEV}/" \
		-e p /srvd/etc/athena/rc.conf > ${ROOT}/${CONFDIR}/rc.conf
else
	conf="`cat /srvd/etc/athena/rc.conf|awk -F= '(NF>1){print $1}'`"
	vars=""
	for i in $conf; do
	   if [ `grep -c "^$i=" ${CONFDIR}/rc.conf` = 0 ]; then 
		vars="$vars $i"
	   fi
	done
	if [ "${vars}" != "" ]; then
	   echo "The following variables are being added:"
	   echo $N "	$C"
	   echo $vars
	   for i in $vars; do \
		grep "^$i=" /srvd/etc/athena/rc.conf >> ${ROOT}${CONFDIR}/rc.conf
	   done
	   echo "done."
	fi
fi

# Make a few hard links to avoid fsck errors
echo $N "Linking Files...$C"
ln ${ROOT}/etc/init ${ROOT}/.deleted/
ln ${ROOT}/bin/sh ${ROOT}/.deleted/ # ! Solaris?
if [ -f ${ROOT}/etc/afsd -a ! -f ${ROOT}/.deleted/etc.afsd ]; then
	ln ${ROOT}/etc/afsd ${ROOT}/.deleted/etc.afsd
fi
if [ -f ${ROOT}/etc/athena/afsd ]; then
	ln ${ROOT}/etc/athena/afsd ${ROOT}/.deleted/etc.athena.afsd
fi
echo "done"

# In case there are serious version incompatibilities...
cp /etc/reboot /tmp/reboot

echo "Removing various old (useless) files..."
cd ${ROOT}/; /bin/rm -f restoresymtable

# We could be more intelligent and shutdown everything, but...
echo $N "Shutting down running services...$C"
case `/bin/athena/machtype` in
sgi)
	killall inetd snmpd syslogd snmpd named
	;;
*)
	if [ -f /etc/inetd.pid ]; then
		echo $N "inetd...$C"
		case $MTYPE in
		POWER*)
			/bin/stopsrc -s inetd
			;;
		*)
			kill `cat /etc/inetd.pid` > /dev/null 2>&1
			;;
		esac
	fi
	if [ -f /etc/athena/inetd.pid ]; then
		echo $N "athena inetd...$C"
		kill `cat /etc/athena/inetd.pid` > /dev/null 2>&1
	fi
	if [ -f /etc/syslog.pid ]; then
		echo $N "syslogd...$C"
		kill `cat /etc/syslog.pid` > /dev/null 2>&1
	fi
	if [ -f /etc/snmpd.pid ]; then
		echo $N "snmpd...$C"
		kill `cat /etc/snmpd.pid` > /dev/null 2>&1
	fi
	if [ -f /etc/named.pid ]; then
		echo $N "named...$C"
		kill `cat /etc/named.pid` > /dev/null 2>&1
	fi
esac
echo "done"

echo "Removing excess kernels..."
/bin/rm -f ${ROOT}/vmunix?*

# Insure /mit is a directory, not a symlink
#rm -f ${ROOT}/mit > /dev/null 2>&1
#mkdir ${ROOT}/mit > /dev/null 2>&1

# Now let us update /etc/ttys
disp="`/bin/athena/machtype -d`"
if [ "${NEWTTYS}" = "true" ]; then
	echo $N "Updating ttys file and /etc/X link...$C"
	case "${disp}" in
	POWER_Gt1)
		sed -e '/^#AIXGTX/s/^#AIXGT//' \
		    -e '/^login/s/^/#/' \
		    -e '/^#AIXGTlogin/s/^#AIXGT//' \
			< /srvd/etc/athena/login/config  \
			> ${ROOT}/etc/athena/login/config
		;;
	PM*)
		dpi=75
		gray=""
		if [ -f /etc/athena/xserver.conf ]; then
			grep -s "^100DPI$" /etc/athena/xserver.conf
			if [ $? = 0 ]; then
				dpi=100
			fi
			grep -s "^Grayscale$" /etc/athena/xserver.conf
			if [ $? = 0 ]; then
				gray="-class GrayScale "
			fi
		fi
		sed -n	-e '/Xws/s/^#//' \
			-e '/^console/s/^/#/' \
			-e '/^#ULTconsole/s/^#ULT//' \
			-e '/^login/s/^/#/' \
			-e '/^#ULT'$dpi'login/s/^#ULT'$dpi'//' \
			-e p /srvd/etc/athena/login/config \
			> ${ROOT}/etc/athena/login/config
		ed - ${ROOT}/etc/athena/login/config << EOF 1>> /dev/null 2>&1
1
/Xws/s/Xws /Xws $gray/
w
q
EOF
		rm -f ${ROOT}/etc/X; ln -s Xws ${ROOT}/etc/X
		;;
	*)
		case "${MACH}" in
		DS*)
			sed -n	-e '/^#console.*getty/s/#//' \
				-e '/^console.*dm /s/^/#/' \
				-e 's/CONSOLE/vt100/' \
				-e p /srvd/etc/ttys > ${ROOT}/etc/ttys
			rm -f ${ROOT}/etc/X
			;;
		RSAIX)
			sed -e '/^#AIXX/s/^#AIX//' \
			    -e '/^login/s/^/#/' \
			    -e '/^#AIXlogin/s/^#AIX//' \
				< /srvd/etc/athena/login/config  \
				> ${ROOT}/etc/athena/login/config
			;;
		esac
		;;
	esac
	echo "done"
fi

echo "Tracking Changes..."
if [ "${FULLCOPY}" = "true" ]; then
	if [ -f ${ROOT}/etc/Xibm ]; then
		echo $N "Removing /etc/Xibm temporarily...$C"
		rm -f ${ROOT}/etc/Xibm
		ln -s /srvd/etc/Xibm ${ROOT}/etc/Xibm
		echo "done."
	fi

	if [ -f ${ROOT}/usr/openwin/bin/Xsun ]; then
		echo $N "Removing /usr/openwin/bin/Xsun temporarily...$C"
		rm -f ${ROOT}/usr/openwin/bin/Xsun
		ln -s /srvd/usr/openwin/bin/Xsun ${ROOT}/usr/openwin/bin/Xsun
		echo "done."
	fi

	if [ "${PUBLIC}" = "true" ]; then RULEDEFS=-DPUBLIC; fi
	RULEDEFS="${RULEDEFS} -D${MACHINE}"

	case ${MACH} in
	RSAIX*)
		sed 's:/\*://**/*:g' /srvd/usr/athena/lib/update/rconf.ws | \
			/usr/lpp/X11/Xamples/util/cpp/cpp ${RULEDEFS} | \
			sed '/^#/s/^/;/' > /tmp/rconf
		synctree -s /srvd -d / -nosrcrules -nodstrules -a /tmp/rconf
		rm -f /tmp/rconf

		echo "Verifying system binaries..."
		/bin/sysck -p inventory
		;;
	SUN*)
		track -c -v -F /srvd -T ${ROOT}/ -d -W /srvd/usr/athena/lib
		;;
	*)
		track -v -F /srvd -T ${ROOT}/ -d -W /srvd/usr/athena/lib
		;;
	esac
fi
case ${MACH} in
	DS*) 
		;;
	RS*)
		;;
	SUN*)
		;;
	INDY)
		;;
	*)
		(cd ${ROOT}/dev; ./MAKEDEV -v ws)
		;;
esac
echo "done."

XSERVER=
XSERVERPATH=/usr/bin/X11
case "${disp}" in
	PM*)
		XSERVER=Xws
		XSERVERPATH=/usr/bin
		XSERVERDEST=/etc
		;;
	8514*|VGA)
		XSERVER=Xibm
		XSERVERPATH=/usr/athena/bin
		XSERVERDEST=/etc
		;;
	SUN*|SPARC*)
		XSERVER=Xsun
		XSERVERPATH=/srvd/usr/openwin/bin
		XSERVERDEST=/usr/openwin/bin
		;;
	*)
		;;
esac

xcopy=true
if [ ${XSERVER}x != x ]; then
	if [ -f /etc/{XSERVER} ]; then
		fout=`find /etc/${XSERVER} \! -newer ${XSERVERPATH}/${XSERVER} -print`
		if [ ${fout}x = x ]; then
			xcopy=
		fi
	fi
	if [ ${xcopy}x = truex ]; then
		echo "Copying X server from" ${XSERVERPATH}/${XSERVER} "to" ${XSERVERDEST}
		rm -f ${XSERVERDEST}/${XSERVER}
		/srvd/bin/cp -p ${XSERVERPATH}/${XSERVER} ${XSERVERDEST}
	fi
fi

# Used to reduce code duplication in NEWUNIX and NEWDEV.
if [ ${MACH} = "SUN4c" ]; then
	cee=.4c
fi

if [ "${NEWUNIX}" = "true" ] ; then
	echo $N "Updating kernel...$C"
	case ${MACH} in
	DS3100)
		/bin/rm -f ${ROOT}/vmunix
		/bin/cp ${CP_P} /srvd/vmunix ${ROOT}/vmunix
		;;
	DS5000*|DSMAXINE)
		/bin/rm -f ${ROOT}/vmunix
		/bin/cp ${CP_P} /srvd/vmunix.5000 ${ROOT}/vmunix
		;;
	DS5400)
		/bin/rm -f ${ROOT}/vmunix
		/bin/cp ${CP_P} /srvd/vmunix.5400 ${ROOT}/vmunix
		;;
	SUN4c|SUN4m)
		(cd /srvd/kernel${cee}; tar cf - . ) | (cd /kernel; tar xf - . )
		(cd /srvd/usr/kernel; tar cf - . ) | (cd /usr/kernel; tar xf - . )
		(cd /srvd/usr/kvm; tar cf - . ) | (cd /usr/kvm; tar xf - . )
		cp -p /srvd/kadb${cee} /kadb
		;;
	*)
		echo "No kernel for this machine."
		;;
	esac
	echo "done."
fi

if [ "${NEWBOOT}" = "true" ] ; then
	echo $N "Copying new bootstraps...$C"
	case $MTYPE in
	KN*)
		cp /srvd/ultrixboot ${ROOT}/
		;;
	SUN*|SPARC*)
		cp -p /srvd/ufsboot ${ROOT}/
		/usr/sbin/installboot /lib/fs/ufs/bootblk /dev/rdsk/c0t3d0s0
		;;
	*)
		echo "No bootstrap for this machine."
		;;
	esac
	echo "done."
fi

echo $N "Updating version...$C"
echo $N "Athena Workstation ($MACH) Version $NEWVERS $C" >> ${ROOT}${CONFDIR}/version
date >> ${CONFDIR}/version
echo "done"

# Re-customize the workstation
if [ "${PUBLIC}" = "true" ]; then
	/bin/rm -rf /${SERVERDIR}
fi

if [ -d ${ROOT}/${SITE}/server ]; then
	/bin/echo "Running mkserv."
	/bin/echo "Restarting named."
	case ${MACH} in
	RS*)
		/etc/athena/named
		;;
	SUN*)
		/usr/sbin/in.named /etc/named.boot
		;;
	INDY)
		/usr/sbin/named `cat $CONFIG/named.options 2> /dev/null` < /dev/null
		;;
	*)
		/etc/named /etc/named.boot
		;;
	esac
	${MKSERV} -v update
fi

if [ "${NEWDEV}" = "true" ] ; then
	/bin/echo "Moving new devices into place"
	case ${MACH} in
	SUN4c|SUN4m)
	        mkdir /devices.77 /dev.77
		  cp /usr/sbin/static/mv /tmp/mv
		  cp /dev/null /reconfigure
		  cd /devices.77; tar xf /srvd/install/tar.devices${cee} .
		  cd /dev.77; tar xf /srvd/install/tar.dev${cee} .
		  /tmp/mv /dev /dev.old
		  /tmp/mv /dev.77 /dev
		  /tmp/mv /devices /devices.old
		  /tmp/mv /devices.77 /devices
		rm -rf /dev.old; rm -rf /devices.old
		  /sbin/sync
		  /sbin/sync
		;;
	*)
		mv /dev /.deleted
		mv /dev.new /dev
	esac
fi

if [ "${NEWOS}" = "true" ] ; then
	/bin/echo "Updating the operating system..."
	/os/update_os
fi

/bin/sync
exit 0


# $Id: Makefile,v 1.15 1999-11-10 03:49:52 kcr Exp $

SHELL=/bin/sh
ETCDIR=/etc
DEFAULT=/etc/default
ATHRETCDIR=/etc/athena
ATHLOGINDIR=${ATHRETCDIR}/login
ATHX11LIB=/usr/athena/lib/X11
APPDEFS=${ATHX11LIB}/app-defaults
X11LIB=/usr/lib/X11
X11APPDEFS=${X11LIB}/app-defaults
FONTS=${X11LIB}/fonts/100dpi
FLEXLM=/var/flexlm
CRONTABS=/var/spool/cron/crontabs
NCS=/var/ncs
SGICONFIG=/etc/config
CONFIG=/afs/athena.mit.edu/system/config
XDMDIR=/var/X11/xdm
AMCONFIGDIR=/var/adm/avail/config

all:

check:

install:
	mkdir -p ${DESTDIR}${ETCDIR}
	mkdir -p ${DESTDIR}${DEFAULT}
	mkdir -p ${DESTDIR}${ATHRETCDIR}
	mkdir -p ${DESTDIR}${SGICONFIG}
	mkdir -p ${DESTDIR}${X11LIB}
	mkdir -p ${DESTDIR}${X11APPDEFS}
	mkdir -p ${DESTDIR}${APPDEFS}
	mkdir -p ${DESTDIR}${FONTS}
	mkdir -p ${DESTDIR}${CRONTABS}
	mkdir -p ${DESTDIR}${FLEXLM}
	mkdir -p ${DESTDIR}${NCS}
	mkdir -p ${DESTDIR}${XDMDIR}
	mkdir -p ${DESTDIR}${AMCONFIGDIR}
	mkdir -p ${DESTDIR}${ATHLOGINDIR}
	install -c -m 0644 ../../aliases ${DESTDIR}${ETCDIR}
	install -c -m 0644 ../../system.mwmrc ${DESTDIR}${X11LIB}
	install -c -m 0644 4DWm ${DESTDIR}${X11APPDEFS}
	install -c -m 0644 Bugme ${DESTDIR}${APPDEFS}
	install -c -m 0644 Console ${DESTDIR}${ATHLOGINDIR}
	install -c -m 0644 config ${DESTDIR}${ATHLOGINDIR}
	install -c -m 0644 Dash ${DESTDIR}${APPDEFS}
	install -c -m 0444 DefaultIconList ${DESTDIR}${XDMDIR}
	install -c -m 0644 Mwm ${DESTDIR}${X11APPDEFS}
	install -c -m 0644 TIMEZONE ${DESTDIR}${ETCDIR}
	install -c -m 0644 Toolchest ${DESTDIR}${X11APPDEFS}
	install -c -m 0644 appletalk ${DESTDIR}${SGICONFIG}
	install -c -m 0644 array ${DESTDIR}${SGICONFIG}
	install -c -m 0644 athinfo.defs ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 attach.conf ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 autoconfig_ipaddress ${DESTDIR}${SGICONFIG}
	install -c -m 0644 autoemail ${DESTDIR}${AMCONFIGDIR}
	install -c -m 0644 autoemail.list ${DESTDIR}${AMCONFIGDIR}
	install -c -m 0644 bsdlpr ${DESTDIR}${SGICONFIG}
	install -c -m 0644 compiler.defaults ${DESTDIR}${ETCDIR}
	install -c -m 0644 crontab.root ${DESTDIR}${CRONTABS}/root
	install -c -m 0644 fixed.pcf ${DESTDIR}${FONTS}
	install -c -m 0644 fonts.alias ${DESTDIR}${FONTS}
	install -c -m 0644 fonts.dir ${DESTDIR}${FONTS}
	install -c -m 0644 group ${DESTDIR}${ETCDIR}
	install -c -m 0644 hinvupdate ${DESTDIR}${AMCONFIGDIR}
	install -c -m 0644 inetd.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 inetd.conf.athena ${DESTDIR}${ATHRETCDIR}/inetd.conf
	install -c -m 0644 livenotification ${DESTDIR}${AMCONFIGDIR}
	install -c -m 0644 lp ${DESTDIR}${SGICONFIG}
	install -c -m 0644 miser ${DESTDIR}${SGICONFIG}
	install -c -m 0644 netwr_client ${DESTDIR}${SGICONFIG}
	install -c -m 0644 newsyslog.conf ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 nsd.options ${DESTDIR}${SGICONFIG}
	install -c -m 0644 nss_fasttrack ${DESTDIR}${SGICONFIG}
	install -c -m 0644 nsswitch.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 passwd.fallback ${DESTDIR}${ETCDIR}
	install -c -m 0644 pcnfsd ${DESTDIR}${SGICONFIG}
	install -c -m 0644 pmcd ${DESTDIR}${SGICONFIG}
	install -c -m 0644 ppagesd ${DESTDIR}${SGICONFIG}
	install -c -m 0644 rc.conf ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 rfindd ${DESTDIR}${SGICONFIG}
	install -c -m 0644 routed ${DESTDIR}${SGICONFIG}
	install -c -m 0644 samba ${DESTDIR}${SGICONFIG}
	install -c -m 0644 sdpd ${DESTDIR}${SGICONFIG}
	install -c -m 0644 sendmail.cf ${DESTDIR}${ETCDIR}
	install -c -m 0644 services ${DESTDIR}${ETCDIR}
	install -c -m 0644 shutdownreason ${DESTDIR}${AMCONFIGDIR}
	install -c -m 0644 snetd ${DESTDIR}${SGICONFIG}
	install -c -m 0644 statusinterval ${DESTDIR}${AMCONFIGDIR}
	install -c -m 0644 su ${DESTDIR}${DEFAULT}
	install -c -m 0644 syslog.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 system.chestrc ${DESTDIR}${X11LIB}
	install -c -m 0644 tickerd ${DESTDIR}${AMCONFIGDIR}
	install -c -m 0644 verbose ${DESTDIR}${SGICONFIG}
	install -c -m 0644 webface ${DESTDIR}${SGICONFIG}
	rm -f ${DESTDIR}${FLEXLM}/license.dat
	ln -s ${CONFIG}/license/sgi/license.dat ${DESTDIR}${FLEXLM}/license.dat
	rm -f ${DESTDIR}${NCS}/glb_site.txt
	ln -s ${CONFIG}/license/sgi/glb_site.txt ${DESTDIR}${NCS}/glb_site.txt
	rm -f ${DESTDIR}${ETCDIR}/shadow
	ln -s ${CONFIG}/passwd/${ATHENA_SYS}/shadow ${DESTDIR}${ETCDIR}/shadow

clean:

distclean:

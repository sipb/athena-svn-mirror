# $Id: Makefile,v 1.2 1997-11-26 21:23:02 cfields Exp $

SHELL=/bin/sh
ETCDIR=/etc
DEFAULT=/etc/default
SGICONFIG=/etc/config
X11LIB=/usr/lib/X11
APPDEFS=${X11LIB}/app-defaults
FONTS=${X11LIB}/fonts/100dpi
CRONTABS=/var/spool/cron/crontabs
NCS=/var/ncs

all:

check:

install:
	-mkdir -p ${DESTDIR}${ETCDIR}
	-mkdir -p ${DESTDIR}${DEFAULT}
	-mkdir -p ${DESTDIR}${X11LIB}
	-mkdir -p ${DESTDIR}${APPDEFS}
	-mkdir -p ${DESTDIR}${FONTS}
	-mkdir -p ${DESTDIR}${CRONTABS}
	-mkdir -p ${DESTDIR}${SGICONFIG}
	install -c -m 0644 4DWm ${DESTDIR}${APPDEFS}
	install -c -m 0644 Toolchest ${DESTDIR}${APPDEFS}
	install -c -m 0644 appletalk ${DESTDIR}${SGICONFIG}
	install -c -m 0644 bsdlpr ${DESTDIR}${SGICONFIG}
	install -c -m 0644 crontab.root ${DESTDIR}${CRONTABS}/root
	install -c -m 0644 fixed.pcf ${DESTDIR}${FONTS}
	install -c -m 0644 fonts.dir ${DESTDIR}${FONTS}
	install -c -m 0644 group ${DESTDIR}${ETCDIR}
	install -c -m 0644 inetd.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 lp ${DESTDIR}${SGICONFIG}
	install -c -m 0644 netwr_client ${DESTDIR}${SGICONFIG}
	install -c -m 0644 ns_fasttrack ${DESTDIR}${SGICONFIG}
	install -c -m 0644 pcnfsd ${DESTDIR}${SGICONFIG}
	install -c -m 0644 services ${DESTDIR}${ETCDIR}
	install -c -m 0644 snetd ${DESTDIR}${SGICONFIG}
	install -c -m 0644 su ${DESTDIR}${DEFAULT}
	install -c -m 0644 system.chestrc ${DESTDIR}${X11LIB}

clean:

distclean:

# $Id: Makefile,v 1.1 1998-02-23 23:33:46 rbasch Exp $

SHELL=/bin/sh
ETCDIR=/etc
DEFAULT=/etc/default
SGICONFIG=/etc/config
X11LIB=/usr/lib/X11
APPDEFS=${X11LIB}/app-defaults
FONTS=${X11LIB}/fonts/100dpi
CRONTABS=/var/spool/cron/crontabs

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
	install -c -m 0644 crontab.root ${DESTDIR}${CRONTABS}/root
	install -c -m 0644 firsttimeprograms ${DESTDIR}${SGICONFIG}
	install -c -m 0644 fixed.pcf ${DESTDIR}${FONTS}
	install -c -m 0644 fonts.dir ${DESTDIR}${FONTS}
	install -c -m 0644 group ${DESTDIR}${ETCDIR}
	install -c -m 0644 inetd.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 lp ${DESTDIR}${SGICONFIG}
	install -c -m 0644 outbox ${DESTDIR}${SGICONFIG}
	install -c -m 0644 pcnfsd ${DESTDIR}${SGICONFIG}
	install -c -m 0644 services ${DESTDIR}${ETCDIR}
	install -c -m 0644 snetd ${DESTDIR}${SGICONFIG}
	install -c -m 0644 su ${DESTDIR}${DEFAULT}
	install -c -m 0644 system.chestrc ${DESTDIR}${X11LIB}
	install -c -m 0644 whatsnew ${DESTDIR}${SGICONFIG}

clean:

distclean:

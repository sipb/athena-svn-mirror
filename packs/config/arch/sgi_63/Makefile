# $Id: Makefile,v 1.1 1997-10-18 04:58:40 ghudson Exp $

SHELL=/bin/sh
ETCDIR=/etc
DEFAULT=/etc/default
CONFIG=/etc/config
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
	install -c -m 0644 4DWm ${DESTDIR}${APPDEFS}
	install -c -m 0644 Toolchest ${DESTDIR}${APPDEFS}
	install -c -m 0644 crontab.root ${DESTDIR}${CRONTABS}/root
	install -c -m 0644 fixed.pcf ${DESTDIR}${FONTS}
	install -c -m 0644 fonts.dir ${DESTDIR}${FONTS}
	install -c -m 0644 group ${DESTDIR}${ETCDIR}
	install -c -m 0644 inetd.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 services ${DESTDIR}${ETCDIR}
	install -c -m 0644 su ${DESTDIR}${DEFAULT}
	install -c -m 0644 system.chestrc ${DESTDIR}${X11LIB}

clean:

distclean:

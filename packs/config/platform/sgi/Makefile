# $Id: Makefile,v 1.4 1997-02-19 18:17:53 ghudson Exp $

SHELL=/bin/sh
ETCDIR=/etc
ATHRETCDIR=/etc/athena
DEFAULT=/etc/default
CONFIG=/etc/config
X11LIB=/usr/lib/X11
APPDEFS=${X11LIB}/app-defaults
FONTS=${X11LIB}/fonts/100dpi
CRONTABS=/var/spool/cron/crontabs
NETLS=/var/netls

all:

check:

install:
	-mkdir -p ${DESTDIR}${ETCDIR}
	-mkdir -p ${DESTDIR}${ATHRETCDIR}
	-mkdir -p ${DESTDIR}${DEFAULT}
	-mkdir -p ${DESTDIR}${CONFIG}
	-mkdir -p ${DESTDIR}${X11LIBS}
	-mkdir -p ${DESTDIR}${APPDEFS}
	-mkdir -p ${DESTDIR}${FONTS}
	-mkdir -p ${DESTDIR}${CRONTABS}
	-mkdir -p ${DESTDIR}${NETLS}
	install -c -m 0644 4DWm ${DESTDIR}${APPDEFS}
	install -c -m 0644 Mwm ${DESTDIR}${APPDEFS}
	install -c -m 0644 TIMEZONE ${DESTDIR}${ETCDIR}
	install -c -m 0644 Toolchest ${DESTDIR}${APPDEFS}
	install -c -m 0644 attach.conf ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 crontab.root ${DESTDIR}${CRONTABS}/root
	install -c -m 0644 fixed.pcf ${DESTDIR}${FONTS}
	install -c -m 0644 fonts.alias ${DESTDIR}${FONTS}
	install -c -m 0644 fonts.dir ${DESTDIR}${FONTS}
	install -c -m 0644 group ${DESTDIR}${ETCDIR}
	install -c -m 0644 inetd.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 inetd.conf.athena ${DESTDIR}${ATHRETCDIR}/inetd.conf
	install -c -m 0644 newsyslog.conf ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 nodelock ${DESTDIR}${NETLS}
	install -c -m 0644 passwd.fallback ${DESTDIR}${ETCDIR}
	install -c -m 0644 ppagesd ${DESTDIR}${CONFIG}
	install -c -m 0644 rc.conf ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 rfindd ${DESTDIR}${CONFIG}
	install -c -m 0644 sendmail.cf ${DESTDIR}${ETCDIR}
	install -c -m 0644 services ${DESTDIR}${ETCDIR}
	install -c -m 0644 staticroute ${DESTDIR}${CONFIG}
	install -c -m 0644 su ${DESTDIR}${DEFAULT}
	install -c -m 0644 system.chestrc ${DESTDIR}${X11LIB}
	install -c -m 0644 syslog.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 verbose ${DESTDIR}${CONFIG}

clean:

distclean:

# $Id: Makefile,v 1.11 1997-10-18 04:55:49 ghudson Exp $

SHELL=/bin/sh
ETCDIR=/etc
ATHRETCDIR=/etc/athena
CONFIG=/etc/config
X11LIB=/usr/lib/X11
APPDEFS=${X11LIB}/app-defaults
FONTS=${X11LIB}/fonts/100dpi
FLEXLM=/var/flexlm
NCS=/var/ncs
SGICONFIG=/etc/config
CONFIG=/afs/athena.mit.edu/system/config

all:

check:

install:
	-mkdir -p ${DESTDIR}${ETCDIR}
	-mkdir -p ${DESTDIR}${ATHRETCDIR}
	-mkdir -p ${DESTDIR}${SGICONFIG}
	-mkdir -p ${DESTDIR}${X11LIB}
	-mkdir -p ${DESTDIR}${APPDEFS}
	-mkdir -p ${DESTDIR}${FONTS}
	-mkdir -p ${DESTDIR}${FLEXLM}
	-mkdir -p ${DESTDIR}${NCS}
	install -c -m 0644 ../../aliases ${DESTDIR}${ETCDIR}
	install -c -m 0644 ../../system.mwmrc ${DESTDIR}${X11LIB}
	install -c -m 0644 Mwm ${DESTDIR}${APPDEFS}
	install -c -m 0644 TIMEZONE ${DESTDIR}${ETCDIR}
	install -c -m 0644 attach.conf ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 fonts.alias ${DESTDIR}${FONTS}
	install -c -m 0644 inetd.conf.athena ${DESTDIR}${ATHRETCDIR}/inetd.conf
	install -c -m 0644 newsyslog.conf ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 passwd.fallback ${DESTDIR}${ETCDIR}
	install -c -m 0644 ppagesd ${DESTDIR}${SGICONFIG}
	install -c -m 0644 rc.conf ${DESTDIR}${ATHRETCDIR}
	install -c -m 0644 rfindd ${DESTDIR}${SGICONFIG}
	install -c -m 0644 sendmail.cf ${DESTDIR}${ETCDIR}
	install -c -m 0644 syslog.conf ${DESTDIR}${ETCDIR}
	install -c -m 0644 verbose ${DESTDIR}${SGICONFIG}
	rm -f ${DESTDIR}${FLEXLM}/license.dat
	ln -s ${CONFIG}/license/sgi/license.dat ${DESTDIR}${FLEXLM}/license.dat
	rm -f ${DESTDIR}${NCS}/glb_site.txt
	ln -s ${CONFIG}/license/sgi/glb_site.txt ${DESTDIR}${NCS}/glb_site.txt

clean:

distclean:

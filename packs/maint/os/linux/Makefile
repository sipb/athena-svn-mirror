# $Id: Makefile,v 1.7.2.1 2001-11-28 22:53:49 ghudson Exp $

SHELL=/bin/sh

SYSCONFDIR=/etc/sysconfig
RCINITDIR=/etc/rc.d/init.d
ATHRETCDIR=/etc/athena
ATHRETCDIR=/etc/athena
ATHLOGINDIR=${ATHRETCDIR}/login

all:

check:

install:
	mkdir -p ${DESTDIR}${SYSCONFDIR} ${DESTDIR}${RCINITDIR}
	mkdir -p ${DESTDIR}${ATHLOGINDIR}
	install -c -m 0755 Xreset ${DESTDIR}${ATHLOGINDIR}
	install -c -m 0755 Xstartup ${DESTDIR}${ATHLOGINDIR}
	install -c -m 0755 athena-ws.rc ${DESTDIR}${RCINITDIR}/athena-ws
	install -c -m 0755 athena-verify.rc \
		${DESTDIR}${RCINITDIR}/athena-verify
	install -c -m 0755 fix-xconfig.sh ${DESTDIR}${ATHRETCDIR}/fix-xconfig
	install -c -m 0755 syncconf.rc ${DESTDIR}${RCINITDIR}/syncconf
	install -c -m 0555 syncconf.sh ${DESTDIR}${ATHRETCDIR}/syncconf
	install -c -m 0755 verify.sh ${DESTDIR}${ATHRETCDIR}/verify
	install -c -m 0755 athena-sysconfig.rc \
		${DESTDIR}${RCINITDIR}/athena-sysconfig

clean:

distclean:

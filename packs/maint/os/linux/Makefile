# $Id: Makefile,v 1.3 2000-04-25 13:51:39 rbasch Exp $

SHELL=/bin/sh

SYSCONFDIR=/etc/sysconfig
RCINITDIR=/etc/rc.d/init.d
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

clean:

distclean:

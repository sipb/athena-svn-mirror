# $Id: Makefile,v 1.1.4.1 2000-02-05 07:18:10 tb Exp $

SHELL=/bin/sh

SYSCONFDIR=/etc/sysconfig
RCINITDIR=/etc/rc.d/init.d

all:

check:

install:
	mkdir -p ${DESTDIR}${SYSCONFDIR} ${DESTDIR}${RCINITDIR}
	install -c -m 0755 athena-ws.rc ${DESTDIR}${RCINITDIR}/athena-ws

clean:

distclean:

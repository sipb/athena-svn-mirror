# $Id: Makefile,v 1.4 2000-04-27 14:25:18 rbasch Exp $

SYSTEM=/usr/athena/lib/init
ETCDIR=/etc
ATHRETCDIR=/etc/athena
ATHLOGINDIR=${ATHRETCDIR}/login

all:

check:

install:
	mkdir -p ${DESTDIR}${SYSTEM}
	mkdir -p ${DESTDIR}${ETCDIR}
	mkdir -p ${DESTDIR}${ATHLOGINDIR}
	install -c -m 0755 ../../Xsession ${DESTDIR}${ATHLOGINDIR}
	install -c -m 0644 cshrc ${DESTDIR}${SYSTEM}
	install -c -m 0644 root.cshrc ${DESTDIR}/.cshrc
	install -c -m 0644 root.login ${DESTDIR}/.login
	install -c -m 0644 root.profile ${DESTDIR}/.profile
	install -c -m 0755 root.xsession ${DESTDIR}/.xsession
	install -c -m 0644 system.login ${DESTDIR}${ETCDIR}/.login
	install -c -m 0644 system.profile ${DESTDIR}${ETCDIR}/profile
	install -c -m 0755 xsession ${DESTDIR}${SYSTEM}

clean:

distclean:

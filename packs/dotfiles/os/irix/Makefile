# $Id: Makefile,v 1.5 1999-11-10 03:51:08 kcr Exp $

SHELL=/bin/sh
SYSTEM=/usr/athena/lib/init
ETCDIR=/etc
ATHRETCDIR=/etc/athena
ATHLOGINDIR=${ATHRETCDIR}/login

all:

check:

install:
	mkdir -p ${DESTDIR}${SYSTEM}
	mkdir -p ${DESTDIR}${ETCDIR}
	mkdir -p ${DESTIDR}${ATHLOGINDIR}
	install -c -m 0755 Xsession ${DESTDIR}${ATHRETCDIR}
	install -c -m 0755 Xsession.dt ${DESTDIR}${SYSTEM}
	install -c -m 0644 cshrc ${DESTDIR}${SYSTEM}
	install -c -m 0644 root.cshrc ${DESTDIR}/.cshrc
	install -c -m 0644 root.login ${DESTDIR}/.login
	install -c -m 0644 root.profile ${DESTDIR}/.profile
	install -c -m 0755 root.xsession ${DESTDIR}/.xsession
	install -c -m 0644 system.cshrc ${DESTDIR}${ETCDIR}/cshrc
	install -c -m 0644 system.profile ${DESTDIR}${ETCDIR}/profile
	install -c -m 0755 xsession ${DESTDIR}${SYSTEM}

clean:

distclean:

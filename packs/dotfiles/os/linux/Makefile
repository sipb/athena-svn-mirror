# $Id: Makefile,v 1.7 2002-08-23 19:41:06 mwhitson Exp $

SYSTEM=/usr/athena/lib/init
ATHRETCDIR=/etc/athena
ATHLOGINDIR=${ATHRETCDIR}/login

all:

check:

install:
	mkdir -p ${DESTDIR}${SYSTEM}
	mkdir -p ${DESTDIR}/root
	mkdir -p ${DESTDIR}${ATHLOGINDIR}
	install -c -m 0755 ../../Xsession ${DESTDIR}${ATHLOGINDIR}
	install -c -m 0644 root.cshrc ${DESTDIR}/root/.cshrc
	install -c -m 0644 root.bashrc ${DESTDIR}/root/.bashrc
	install -c -m 0644 root.login ${DESTDIR}/root/.login
	install -c -m 0644 root.profile ${DESTDIR}/root/.profile
	install -c -m 0755 root.xsession ${DESTDIR}/root/.xsession
	install -c -m 0755 xsession ${DESTDIR}${SYSTEM}

clean:

distclean:

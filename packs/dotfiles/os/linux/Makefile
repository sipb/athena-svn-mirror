# $Id: Makefile,v 1.3 2000-02-28 16:32:33 tb Exp $

SYSTEM=/usr/athena/lib/init
ATHRETCDIR=/etc/athena
ATHLOGINDIR=${ATHRETCDIR}/login

all:

check:

install:
	mkdir -p ${DESTDIR}${SYSTEM}
	mkdir -p ${DESTDIR}/root
	mkdir -p ${DESTDIR}${ATHRETCDIR}
	install -c -m 0755 ../../Xsession ${DESTDIR}${THRETCDIR}
	install -c -m 0644 cshrc ${DESTDIR}${SYSTEM}
	install -c -m 0644 root.cshrc ${DESTDIR}/root/.cshrc
	install -c -m 0644 root.login ${DESTDIR}/root/.login
	install -c -m 0644 root.profile ${DESTDIR}/root/.profile
	install -c -m 0755 root.xsession ${DESTDIR}/root/.xsession
	install -c -m 0755 xsession ${DESTDIR}${SYSTEM}

clean:

distclean:

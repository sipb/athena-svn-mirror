# $Id: Makefile,v 1.2 1999-11-10 03:51:29 kcr Exp $

SYSTEM=/usr/athena/lib/init
ATHRETCDIR=/etc/athena
ATHLOGINDIR=${ATHRETCDIR}/login

all:

check:

install:
	mkdir -p ${DESTDIR}${SYSTEM}
	mkdir -p ${DESTDIR}${ATHRETCDIR}
	install -c -m 0755 ../../Xsession ${DESTDIR}${THRETCDIR}
	install -c -m 0644 cshrc ${DESTDIR}${SYSTEM}
	install -c -m 0644 root.cshrc ${DESTDIR}/.cshrc
	install -c -m 0644 root.login ${DESTDIR}/.login
	install -c -m 0644 root.profile ${DESTDIR}/.profile
	install -c -m 0755 root.xsession ${DESTDIR}/.xsession
	install -c -m 0755 xsession ${DESTDIR}${SYSTEM}

clean:

distclean:

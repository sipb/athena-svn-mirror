# $Id: Makefile,v 1.1 1999-06-11 15:27:09 tb Exp $

SYSTEM=/usr/athena/lib/init

all:

check:

install:
	mkdir -p ${DESTDIR}${SYSTEM}
	install -c -m 0644 cshrc ${DESTDIR}${SYSTEM}
	install -c -m 0644 root.cshrc ${DESTDIR}/.cshrc
	install -c -m 0644 root.login ${DESTDIR}/.login
	install -c -m 0644 root.profile ${DESTDIR}/.profile
	install -c -m 0755 root.xsession ${DESTDIR}/.xsession
	install -c -m 0755 xsession ${DESTDIR}${SYSTEM}

clean:

distclean:

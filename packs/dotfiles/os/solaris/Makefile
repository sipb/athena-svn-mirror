# $Id: Makefile,v 1.8 2005-04-22 18:25:04 ghudson Exp $

ETCDIR=/etc

all:

check:

install:
	mkdir -p ${DESTDIR}${ETCDIR}
	install -c -m 0644 root.bashrc ${DESTDIR}/.bashrc
	install -c -m 0644 root.cshrc ${DESTDIR}/.cshrc
	install -c -m 0644 root.login ${DESTDIR}/.login
	install -c -m 0644 root.profile ${DESTDIR}/.profile
	install -c -m 0755 root.xsession ${DESTDIR}/.xsession
	install -c -m 0644 system.cshrc ${DESTDIR}${ETCDIR}/.cshrc
	install -c -m 0644 system.login ${DESTDIR}${ETCDIR}/.login
	install -c -m 0644 system.profile ${DESTDIR}${ETCDIR}/profile

clean:

distclean:

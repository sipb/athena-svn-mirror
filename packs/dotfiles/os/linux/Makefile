# $Id: Makefile,v 1.8 2002-08-29 19:45:27 mwhitson Exp $

all:

check:

install:
	mkdir -p ${DESTDIR}/root
	install -c -m 0644 root.cshrc ${DESTDIR}/root/.cshrc
	install -c -m 0644 root.bashrc ${DESTDIR}/root/.bashrc
	install -c -m 0644 root.login ${DESTDIR}/root/.login
	install -c -m 0644 root.profile ${DESTDIR}/root/.profile
	install -c -m 0755 root.xsession ${DESTDIR}/root/.xsession

clean:

distclean:

# $Id: Makefile,v 1.9 2005-04-26 20:28:21 ghudson Exp $

all:

check:

install:
	mkdir -p ${DESTDIR}/root
	install -c -m 0644 root.cshrc ${DESTDIR}/root/.cshrc
	install -c -m 0644 root.bashrc ${DESTDIR}/root/.bashrc
	install -c -m 0644 root.bash_login ${DESTDIR}/root/.bash_login
	install -c -m 0644 root.login ${DESTDIR}/root/.login
	install -c -m 0644 root.profile ${DESTDIR}/root/.profile
	install -c -m 0755 root.xsession ${DESTDIR}/root/.xsession

clean:

distclean:

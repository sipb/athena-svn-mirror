# $Id: Makefile,v 1.1.2.1 1997-11-04 18:58:47 ghudson Exp $

SHELL=/bin/sh
SYSTEM=/usr/athena/lib/init

all:

check:

install:
	-mkdir -p ${DESTDIR}${SYSTEM}
	install -c -m 0644 cshrc ${DESTDIR}${SYSTEM}
	install -c -m 0755 Xsession.dt ${DESTDIR}${SYSTEM}

clean:

distclean:

# $Id: Makefile,v 1.1 1997-10-18 05:00:22 ghudson Exp $

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

# $Id: Makefile,v 1.2 1998-12-26 15:51:49 ghudson Exp $

SHELL=/bin/sh
INITD=/etc/init.d
ATHRETCDIR=/etc/athena

all:

check:

install:
	mkdir -p ${DESTDIR}${INITD}
	mkdir -p ${DESTDIR}${ATHRETCDIR}
	install -c -m 0755 network ${DESTDIR}${INITD}
	install -c -m 0555 syncconf.sh ${DESTDIR}${ATHRETCDIR}/syncconf

clean:

distclean:

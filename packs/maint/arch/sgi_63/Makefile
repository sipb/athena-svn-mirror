# $Id: Makefile,v 1.2.2.1 1997-11-04 18:58:41 ghudson Exp $

SHELL=/bin/sh
INITD=/etc/init.d
ATHRETCDIR=/etc/athena

all:

check:

install:
	-mkdir -p ${DESTDIR}${INITD}
	-mkdir -p ${DESTDIR}${ATHRETCDIR}
	install -c -m 0755 network ${DESTDIR}${INITD}
	install -c -m 0555 syncconf.sh ${DESTDIR}${ATHRETCDIR}/syncconf

clean:

distclean:

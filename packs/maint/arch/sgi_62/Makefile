# $Id: Makefile,v 1.1 1998-02-25 18:46:57 rbasch Exp $

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

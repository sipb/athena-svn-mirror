# $Id: Makefile,v 1.4 1999-08-12 15:43:24 danw Exp $

SHELL=/bin/sh
USRBIN=/usr/bin
USRSBIN=/usr/sbin

all:

check:

install:
	mkdir -p ${DESTDIR}${USRBIN}
	mkdir -p ${DESTDIR}${USRSBIN}
	rm -f ${DESTDIR}/bin
	ln -s usr/bin ${DESTDIR}/bin
	install -c -m 755 postinst ${DESTDIR}${USRSBIN}

clean:

distclean:

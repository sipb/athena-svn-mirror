# $Id: Makefile,v 1.6 2000-10-12 22:28:41 rbasch Exp $

SHELL=/bin/sh
USRBIN=/usr/bin
USRSBIN=/usr/sbin
DEVLIB=/usr/lib/devicelib

all:

check:

install:
	mkdir -p ${DESTDIR}${USRBIN}
	mkdir -p ${DESTDIR}${USRSBIN}
	rm -f ${DESTDIR}/bin
	ln -s usr/bin ${DESTDIR}/bin
	install -c -m 755 postinst ${DESTDIR}${USRSBIN}
	mkdir -p ${DESTDIR}/usr/local/lib/netscape/plugins
	rm -f ${DESTDIR}/usr/local/lib/netscape/plugins/libcosmoplayer.so
	ln -s \
	  ../../../../../var/netscape/communicator/plugins/libcosmoplayer.so \
	  ${DESTDIR}/usr/local/lib/netscape/plugins/libcosmoplayer.so
	mkdir -p ${DESTDIR}${DEVLIB}
	install -c -m 755 dev_zip250.so ${DESTDIR}${DEVLIB}

clean:

distclean:

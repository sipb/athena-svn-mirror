# $Id: Makefile,v 1.7 2003-07-24 18:48:42 ghudson Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
USRBINDIR=/usr/bin

all:

check:

install:
	mkdir -p ${DESTDIR}${ATHBINDIR}
	mkdir -p ${DESTDIR}${USRBINDIR}
	install -c -m 555 gnome-panel.sh ${DESTDIR}${ATHBINDIR}/gnome-panel
	install -c -m 555 lp.sh ${DESTDIR}${ATHBINDIR}/lp
	rm -f ${DESTDIR}${USRBINDIR}/lp
	ln -s ../athena/bin/lp ${DESTDIR}${USRBINDIR}/lp
	install -c -m 555 whatis.sh ${DESTDIR}${ATHBINDIR}/whatis
	rm -f ${DESTDIR}${ATHBINDIR}/apropos
	ln -s whatis ${DESTDIR}${ATHBINDIR}/apropos

clean:

distclean:

# $Id: Makefile,v 1.1 1996-09-20 05:01:56 ghudson Exp $

INSTALL=/install

all:

check:

install:
	mkdir -p ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.SUN0207 ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.SUN0424 ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.SUN0535 ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.SUN1.05 ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.seagate.5660 ${DESTDIR}${INSTALL}
	install -c -m 0444 phase2.sh ${DESTDIR}${INSTALL}
	install -c -m 0444 phase3.sh ${DESTDIR}${INSTALL}
	install -c -m 0444 startup ${DESTDIR}${INSTALL}
	install -c -m 0444 athenainstall ${DESTDIR}${INSTALL}
	install -c -m 0444 var.cpio ${DESTDIR}${INSTALL}

clean:

distclean:

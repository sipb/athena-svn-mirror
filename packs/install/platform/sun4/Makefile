# $Id: Makefile,v 1.3 1997-09-11 18:32:58 ghudson Exp $

INSTALL=/install

all:

check:

install:
	mkdir -p ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.SUN0424 ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.SUN0535 ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.SUN1.05 ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.seagate.5660 ${DESTDIR}${INSTALL}
	install -c -m 0444 phase2.sh ${DESTDIR}${INSTALL}
	install -c -m 0444 phase3.sh ${DESTDIR}${INSTALL}
	install -c -m 0444 startup ${DESTDIR}${INSTALL}
	install -c -m 0444 athenainstall ${DESTDIR}${INSTALL}

clean:

distclean:

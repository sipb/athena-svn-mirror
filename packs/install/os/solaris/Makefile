# $Id: Makefile,v 1.3 1999-01-12 15:48:08 miki Exp $

INSTALL=/install
INSTALL_DEV=/install/devices

all:

check:

install:
	mkdir -p ${DESTDIR}${INSTALL}
	mkdir -p ${DESTDIR}${INSTALL_DEV}
	install -c -m 0444 format.input.SUN0424 ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.SUN0535 ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.SUN1.05 ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.ST34321A ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.ST34342A ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.ST39140A ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.label ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.seagate.5660 ${DESTDIR}${INSTALL}
	install -c -m 0444 phase2.sh ${DESTDIR}${INSTALL}
	install -c -m 0444 phase3.sh ${DESTDIR}${INSTALL}
	install -c -m 0444 startup ${DESTDIR}${INSTALL}
	install -c -m 0444 athenainstall ${DESTDIR}${INSTALL}
	install -c -m 0444 dev.pseudo.tar ${DESTDIR}${INSTALL_DEV}
	install -c -m 0444 devices.pseudo.tar ${DESTDIR}${INSTALL_DEV}

clean:

distclean:

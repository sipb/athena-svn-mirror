# $Id: Makefile,v 1.17 2004-05-25 17:06:05 rbasch Exp $

INSTALL=/install
INSTALL_DEV=${INSTALL}/devices

all:	fastfs to

fastfs:	fastfs.c
	${CC} ${LDFLAGS} -o $@ fastfs.c

to:	to.c
	${CC} ${LDFLAGS} -o $@ to.c

check:

install:
	mkdir -p ${DESTDIR}${INSTALL}
	mkdir -p ${DESTDIR}${INSTALL_DEV}
	install -c -m 0555 fastfs ${DESTDIR}${INSTALL}
	install -c -m 0555 finish-install.sh \
	  ${DESTDIR}${INSTALL}/finish-install
	install -c -m 0555 finish-install-wrapper.sh \
	  ${DESTDIR}${INSTALL}/finish-install-wrapper
	install -c -m 0444 format.input.SEAGATE-ST39173WC-6244 ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.ST315320A ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.ST320011A ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.ST320413A ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.ST320420A ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.ST34321A ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.ST34342A ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.ST340824A ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.ST38420A ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.ST39120A ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.ST39140A ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.SUN0424 ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.SUN0535 ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.SUN1.05 ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.SUN18G ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.SUN2.1G ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.SUN36G ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.SUN4.2G ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.SUN9.0G ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.label ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.seagate.5660 ${DESTDIR}${INSTALL}
	install -c -m 0444 format.input.Seagate.medalist ${DESTDIR}${INSTALL}
	install -c -m 0444 install1.sh ${DESTDIR}${INSTALL}
	install -c -m 0444 install2.sh ${DESTDIR}${INSTALL}
	install -c -m 0555 oschanges ${DESTDIR}${INSTALL}
	install -c -m 0444 os-link-2.6 ${DESTDIR}${INSTALL}
	install -c -m 0555 startup ${DESTDIR}${INSTALL}
	install -c -m 0555 to ${DESTDIR}${INSTALL}
	install -c -m 0444 athenainstall ${DESTDIR}${INSTALL}
	install -c -m 0444 dev.pseudo.tar ${DESTDIR}${INSTALL_DEV}
	install -c -m 0444 devices.pseudo.tar ${DESTDIR}${INSTALL_DEV}

clean:

distclean:

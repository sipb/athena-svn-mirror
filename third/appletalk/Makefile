# $Id: Makefile,v 1.2 1997-01-27 13:34:42 ghudson Exp $

SHELL=/bin/sh
AUXDEVS=/install/aux.devs
DRV=/kernel/drv
STRMOD=/kernel/strmod

all:

check:

install:
	mkdir -p ${DESTDIR}${AUXDEVS}
	mkdir -p ${DESTDIR}${DRV}
	mkdir -p ${DESTDIR}${STRMOD}
	install -c -m 0555 adsp ${DESTDIR}${STRMOD}
	install -c -m 0555 at_atp ${DESTDIR}${STRMOD}
	install -c -m 0555 at_sig ${DESTDIR}${STRMOD}
	install -c -m 0555 appletalk.sh ${DESTDIR}${AUXDEVS}/appletalk
	install -c -m 0555 atalk ${DESTDIR}${DRV}
	install -c -m 0444 atalk.conf ${DESTDIR}${DRV}

clean:

distclean:

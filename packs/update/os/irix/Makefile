# $Id: Makefile,v 1.3.2.1 2001-08-21 00:25:32 ghudson Exp $

SHELL=/bin/sh
SLISTS=/usr/athena/lib/slists
UPDATE=/usr/athena/lib/update

all:

check:

install:
	mkdir -p ${DESTDIR}${UPDATE}
	mkdir -p ${DESTDIR}${SLISTS}
	install -c -m 0444 configfiles ${DESTDIR}${UPDATE}
	install -c -m 0444 sys_rvd ${DESTDIR}${SLISTS}
	install -c -m 0555 setup-swap-boot.sh \
		${DESTDIR}${UPDATE}/setup-swap-boot
	install -c -m 0555 update-os.sh ${DESTDIR}${UPDATE}/update-os

clean:

distclean:

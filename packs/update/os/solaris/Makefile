# $Id: Makefile,v 1.3 2001-04-12 17:14:26 ghudson Exp $

SLISTS=/usr/athena/lib/slists
UPDATE=/usr/athena/lib/update

all:

check:

install:
	mkdir -p ${DESTDIR}${UPDATE}
	mkdir -p ${DESTDIR}${SLISTS}
	install -c -m 0444 configfiles ${DESTDIR}${UPDATE}
	install -c -m 0444 kernel_rvd ${DESTDIR}${SLISTS}
	install -c -m 0444 os_rvd ${DESTDIR}${SLISTS}
	install -c -m 0555 setup-swap-boot.sh \
		${DESTDIR}${UPDATE}/setup-swap-boot
	install -c -m 0444 sys_rvd ${DESTDIR}${SLISTS}
	install -c -m 0444 sys_rvd.local ${DESTDIR}${SLISTS}
	install -c -m 0555 update-os.sh ${DESTDIR}${UPDATE}/update-os
	install -c -m 0444 usr_kernel_rvd ${DESTDIR}${SLISTS}

clean:

distclean:

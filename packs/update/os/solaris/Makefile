# $Id: Makefile,v 1.1 1998-12-25 19:34:03 ghudson Exp $

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
	install -c -m 0444 sys_rvd ${DESTDIR}${SLISTS}
	install -c -m 0555 update-os.sh ${DESTDIR}${UPDATE}/update-os
	install -c -m 0444 usr_kernel_rvd ${DESTDIR}${SLISTS}

clean:

distclean:

# $Id: Makefile,v 1.7 2004-03-31 00:13:23 rbasch Exp $

SLISTS=/usr/athena/lib/slists
UPDATE=/usr/athena/lib/update

all:

check:

install:
	mkdir -p ${DESTDIR}${UPDATE}
	mkdir -p ${DESTDIR}${SLISTS}
	install -c -m 0444 admin-update ${DESTDIR}${UPDATE}
	install -c -m 0444 configfiles ${DESTDIR}${UPDATE}
	install -c -m 0555 fix-partitions.sh ${DESTDIR}${UPDATE}/fix-partitions
	install -c -m 0444 kernel_rvd ${DESTDIR}${SLISTS}
	install -c -m 0444 os_rvd ${DESTDIR}${SLISTS}
	install -c -m 0555 pkg-order.pl ${DESTDIR}${UPDATE}/pkg-order
	install -c -m 0444 sys_rvd ${DESTDIR}${SLISTS}
	install -c -m 0444 sys_rvd.big ${DESTDIR}${SLISTS}
	install -c -m 0555 track-srvd.sh ${DESTDIR}${UPDATE}/track-srvd
	install -c -m 0555 update-os.sh ${DESTDIR}${UPDATE}/update-os
	install -c -m 0444 usr_kernel_rvd ${DESTDIR}${SLISTS}

clean:

distclean:

# $Id: Makefile,v 1.1 1996-09-20 05:02:42 ghudson Exp $

SLISTS=/usr/athena/lib/slists

all:

check:

install:
	mkdir -p ${DESTDIR}${SLISTS}
	install -c -m 0444 kernel_rvd ${DESTDIR}${SLISTS}
	install -c -m 0444 os_rvd ${DESTDIR}${SLISTS}
	install -c -m 0444 sys_rvd ${DESTDIR}${SLISTS}
	install -c -m 0444 usr_kernel_rvd ${DESTDIR}${SLISTS}

clean:

distclean:

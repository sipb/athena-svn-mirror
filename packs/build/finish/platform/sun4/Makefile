# $Id: Makefile,v 1.1 1996-09-20 05:14:28 ghudson Exp $

SHELL=/bin/sh
ATHETCDIR=/usr/athena/etc
ATHLIBDIR=/usr/athena/lib
ATHMANDIR=/usr/athena/man

all:

check:

install:
	${DESTDIR}${ATHETCDIR}/track -w -F/os -W${DESTDIR}${ATHLIBDIR} \
		-s stats/os_rvd slists/os_rvd
	${DESTDIR}${ATHETCDIR}/track -w -F/os/kernel -W${DESTDIR}${ATHLIBDIR} \
		-s stats/kernel_rvd slists/kernel_rvd
	${DESTDIR}${ATHETCDIR}/track -w -F/os/usr/kernel \
		-W${DESTDIR}${ATHLIBDIR} -s stats/usr_kernel_rvd \
		slists/usr_kernel_rvd
	catman -M ${DESTDIR}${ATHMANDIR}
	chown 0 ${DESTDIR}${ATHMANDIR}/windex
	chgrp 0 ${DESTDIR}${ATHMANDIR}/windex

clean:

distclean:

# $Id: Makefile,v 1.2 1996-12-27 22:14:17 ghudson Exp $

SHELL=/bin/sh
SLISTS=/usr/athena/lib/slists
UPDATE=/usr/athena/lib/update

all:

check:

install:
	-mkdir -p ${DESTDIR}${UPDATE}
	-mkdir -p ${DESTDIR}${SLISTS}
	install -c -m 0444 configfiles ${DESTDIR}${UPDATE}
	install -c -m 0444 sys_rvd ${DESTDIR}${SLISTS}

clean:

distclean:

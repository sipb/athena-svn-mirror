# $Id: Makefile,v 1.1.2.1 1999-06-18 21:54:35 ghudson Exp $

SLISTS=/usr/athena/lib/slists
UPDATE=/usr/athena/lib/update

all:

check:

install:
	mkdir -p ${DESTDIR}${UPDATE}
	mkdir -p ${DESTDIR}${SLISTS}
	install -c -m 0444 configfiles ${DESTDIR}${UPDATE}
	install -c -m 0444 sys_rvd ${DESTDIR}${SLISTS}

clean:

distclean:

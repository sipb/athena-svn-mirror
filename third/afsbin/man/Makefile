# $Id: Makefile,v 1.2 1996-12-18 16:58:05 ghudson Exp $

SHELL=/bin/sh
ATHMANDIR=/usr/athena/man

all:

check:

install:
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man1
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man2
	install -c -m 444 bos.1 ${DESTDIR}${ATHMANDIR}/man1/bos.1
	install -c -m 444 fs.1 ${DESTDIR}${ATHMANDIR}/man1/fs.1
	install -c -m 444 pts.1 ${DESTDIR}${ATHMANDIR}/man1/pts.1
	install -c -m 444 tokens.1 ${DESTDIR}${ATHMANDIR}/man1/tokens.1
	install -c -m 444 unlog.1 ${DESTDIR}${ATHMANDIR}/man1/unlog.1
	install -c -m 444 vos.1 ${DESTDIR}${ATHMANDIR}/man1/vos.1
	install -c -m 444 icreate.2 ${DESTDIR}${ATHMANDIR}/man2/icreate.2
	install -c -m 444 iinc.2 ${DESTDIR}${ATHMANDIR}/man2/iinc.2
	install -c -m 444 iopen.2 ${DESTDIR}${ATHMANDIR}/man2/iopen.2
	install -c -m 444 iread.2 ${DESTDIR}${ATHMANDIR}/man2/iread.2
	install -c -m 444 iwrite.2 ${DESTDIR}${ATHMANDIR}/man2/iwrite.2
	install -c -m 444 pioctl.2 ${DESTDIR}${ATHMANDIR}/man2/pioctl.2
	install -c -m 444 setpag.2 ${DESTDIR}${ATHMANDIR}/man2/setpag.2

clean:

distclean:

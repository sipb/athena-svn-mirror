# $Id: Makefile,v 1.2 1996-10-11 17:26:01 ghudson Exp $

SHELL=/bin/sh
ATHBINDIR=/usr/athena/bin
ATHMANDIR=/usr/athena/man

all: look

look: look.o
	${CC} ${LDFLAGS} -o look look.o ${LIBS}

check:

install:
	-mkdir -p ${DESTDIR}${ATHBINDIR}
	-mkdir -p ${DESTDIR}${ATHMANDIR}/man1
	install -c -m 555 look ${DESTDIR}${ATHBINDIR}
	install -c -m 444 look.1 ${DESTDIR}${ATHMANDIR}/man1

clean:
	rm -f look.o look

distclean: clean

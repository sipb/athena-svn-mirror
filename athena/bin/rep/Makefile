#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/rep/Makefile,v $
#	$Author: builder $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/rep/Makefile,v 1.2 1985-06-17 13:17:12 builder Exp $
#
DESTDIR=
INCDIR=/usr/include
CFLAGS=-O -I${INCDIR}

all: rep

rep: rep.c
	cc ${CFLAGS} rep.c -lcurses -ltermlib -o rep
install: rep
	install -c -s rep ${DESTDIR}/usr/athena/rep

clean:
	rm -f core rep

#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/rep/Makefile,v $
#	$Author: builder $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/rep/Makefile,v 1.4 1985-11-06 20:46:31 builder Exp $
#
DESTDIR=
PHYSLOC=/u1
INCDIR=/usr/include
CFLAGS=-O -I${INCDIR}

all: rep

rep: rep.c
	cc ${CFLAGS} rep.c -lcurses -ltermlib -o rep
install: rep
	install -c -s rep ${DESTDIR}${PHYSLOC}/usr/athena/rep
	install -c rep.1 ${DESTDIR}${PHYSLOC}/usr/man/mann/rep.n

clean:
	rm -f core rep

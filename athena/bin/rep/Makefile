#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/rep/Makefile,v $
#	$Author: builder $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/rep/Makefile,v 1.3 1985-06-17 14:34:11 builder Exp $
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

clean:
	rm -f core rep

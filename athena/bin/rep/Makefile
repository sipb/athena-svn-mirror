#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/rep/Makefile,v $
#	$Author: builder $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/rep/Makefile,v 1.5 1985-11-13 21:22:05 builder Exp $
#
DESTDIR=
CFLAGS=-O 

all: rep

rep: rep.c
	cc ${CFLAGS} rep.c -lcurses -ltermlib -o rep
install: rep
	install -c -s rep ${DESTDIR}/usr/athena/rep
	install -c rep.1 ${DESTDIR}/usr/man/mann/rep.n

clean:
	rm -f core rep

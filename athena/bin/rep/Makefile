#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/rep/Makefile,v $
#	$Author: epeisach $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/rep/Makefile,v 1.6 1989-09-16 12:50:38 epeisach Exp $
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

depend:
	makedepend ${CFLAGS} rep.c

# DO NOT DELETE THIS LINE -- make depend depends on it.

rep.o: rep.c /usr/include/curses.h /usr/include/stdio.h /usr/include/sgtty.h
rep.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
rep.o: /usr/include/sys/ttydev.h /usr/include/signal.h

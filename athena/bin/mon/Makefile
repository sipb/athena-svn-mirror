#
# 	$Source: /afs/dev.mit.edu/source/repository/athena/bin/mon/Makefile,v $
#	$Author: probe $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/mon/Makefile,v 1.8 1990-05-01 09:11:03 probe Exp $
#


DESTDIR=
# mon makefile
#
#  Beware dependencies on mon.h are not properly stated.
#
CFLAGS = -O

OBJS = mon.o io.o vm.o netif.o display.o readnames.o

all: mon

mon: $(OBJS) mon.h
	cc -o mon $(OBJS) -lcurses -ltermlib

install:
	install -c -s -g kmem -m 2755 mon ${DESTDIR}/usr/athena/mon

clean:
	rm -f core *.o mon a.out *~

print:
	qpr mon.h mon.c io.c vm.c netif.c readnames.c display.c

depend:
	touch Make.depend; makedepend -fMake.depend ${CFLAGS} mon.c io.c vm.c netif.c readnames.c display.c


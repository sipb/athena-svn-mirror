#
# 	$Source: /afs/dev.mit.edu/source/repository/athena/bin/mon/Makefile,v $
#	$Author: builder $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/mon/Makefile,v 1.4 1985-10-15 20:16:38 builder Exp $
#


DESTDIR=
PHYSLOC=
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
	install -c -s -g kmem -m 2755 mon ${DESTDIR}${PHYSLOC}/usr/athena/mon

clean:
	rm -f core *.o mon a.out

print:
	qpr mon.h mon.c io.c vm.c netif.c readnames.c display.c

#
# 	$Source: /afs/dev.mit.edu/source/repository/athena/bin/mon/Makefile,v $
#	$Author: dgg $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/mon/Makefile,v 1.2 1984-12-13 12:10:16 dgg Exp $
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
	install -c -s -g memory -m 2755 mon ${DESTDIR}/etc/athena/mon

clean:
	rm -f core *.o mon a.out

print:
	qpr mon.h mon.c io.c vm.c netif.c readnames.c display.c

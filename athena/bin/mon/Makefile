#
# 	$Source: /afs/dev.mit.edu/source/repository/athena/bin/mon/Makefile,v $
#	$Author: epeisach $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/mon/Makefile,v 1.6 1989-09-16 12:45:34 epeisach Exp $
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
	makedepend ${CFLAGS} mon.c io.c vm.c netif.c readnames.c display.c

# DO NOT DELETE THIS LINE -- make depend depends on it.

mon.o: mon.c mon.h /usr/include/stdio.h /usr/include/ctype.h
mon.o: /usr/include/nlist.h /usr/include/sys/types.h /usr/include/sys/vm.h
mon.o: /usr/include/sys/vmparam.h /usr/include/machine/vmparam.h
mon.o: /usr/include/sys/vmmac.h /usr/include/sys/vmmeter.h
mon.o: /usr/include/sys/vmsystm.h /usr/include/sys/dk.h
mon.o: /usr/include/sys/time.h /usr/include/sys/time.h /usr/include/curses.h
mon.o: /usr/include/sgtty.h /usr/include/sys/ioctl.h
mon.o: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
mon.o: /usr/include/signal.h
io.o: io.c mon.h /usr/include/stdio.h /usr/include/ctype.h
io.o: /usr/include/nlist.h /usr/include/sys/types.h /usr/include/sys/vm.h
io.o: /usr/include/sys/vmparam.h /usr/include/machine/vmparam.h
io.o: /usr/include/sys/vmmac.h /usr/include/sys/vmmeter.h
io.o: /usr/include/sys/vmsystm.h /usr/include/sys/dk.h
io.o: /usr/include/sys/time.h /usr/include/sys/time.h /usr/include/curses.h
io.o: /usr/include/sgtty.h /usr/include/sys/ioctl.h
io.o: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
vm.o: vm.c mon.h /usr/include/stdio.h /usr/include/ctype.h
vm.o: /usr/include/nlist.h /usr/include/sys/types.h /usr/include/sys/vm.h
vm.o: /usr/include/sys/vmparam.h /usr/include/machine/vmparam.h
vm.o: /usr/include/sys/vmmac.h /usr/include/sys/vmmeter.h
vm.o: /usr/include/sys/vmsystm.h /usr/include/sys/dk.h
vm.o: /usr/include/sys/time.h /usr/include/sys/time.h
vm.o: /usr/include/machine/machparam.h
netif.o: netif.c mon.h /usr/include/stdio.h /usr/include/ctype.h
netif.o: /usr/include/nlist.h /usr/include/sys/types.h /usr/include/sys/vm.h
netif.o: /usr/include/sys/vmparam.h /usr/include/machine/vmparam.h
netif.o: /usr/include/sys/vmmac.h /usr/include/sys/vmmeter.h
netif.o: /usr/include/sys/vmsystm.h /usr/include/sys/dk.h
netif.o: /usr/include/sys/time.h /usr/include/sys/time.h
netif.o: /usr/include/sys/socket.h /usr/include/net/if.h
netif.o: /usr/include/net/if_arp.h /usr/include/netinet/in.h
readnames.o: readnames.c mon.h /usr/include/stdio.h /usr/include/ctype.h
readnames.o: /usr/include/nlist.h /usr/include/sys/types.h
readnames.o: /usr/include/sys/vm.h /usr/include/sys/vmparam.h
readnames.o: /usr/include/machine/vmparam.h /usr/include/sys/vmmac.h
readnames.o: /usr/include/sys/vmmeter.h /usr/include/sys/vmsystm.h
readnames.o: /usr/include/sys/dk.h /usr/include/sys/time.h
readnames.o: /usr/include/sys/time.h /usr/include/sys/buf.h
readnames.o: /usr/include/vaxuba/ubavar.h /usr/include/vaxmba/mbavar.h
display.o: display.c mon.h /usr/include/stdio.h /usr/include/ctype.h
display.o: /usr/include/nlist.h /usr/include/sys/types.h
display.o: /usr/include/sys/vm.h /usr/include/sys/vmparam.h
display.o: /usr/include/machine/vmparam.h /usr/include/sys/vmmac.h
display.o: /usr/include/sys/vmmeter.h /usr/include/sys/vmsystm.h
display.o: /usr/include/sys/dk.h /usr/include/sys/time.h
display.o: /usr/include/sys/time.h /usr/include/curses.h /usr/include/sgtty.h
display.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
display.o: /usr/include/sys/ttydev.h

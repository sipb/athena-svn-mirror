# $Header: /afs/dev.mit.edu/source/repository/athena/etc/cleanup/Makefile,v 1.3 1990-11-23 08:29:07 probe Exp $

GCC = `if [ "${MACHINE}" = "vax" ]; then echo gcc; else echo cc; fi`
CFLAGS = -O

all: cleanup

cleanup: cleanup.o
	$(CC) -o cleanup cleanup.o -lhesiod

cleanup.o: cleanup.c
	$(GCC) $(CFLAGS) -c cleanup.c

clean:
	rm -f *.o cleanup

install: cleanup
	install -c -s cleanup $(DESTDIR)/etc/athena/cleanup

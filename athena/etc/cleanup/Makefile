# $Header: /afs/dev.mit.edu/source/repository/athena/etc/cleanup/Makefile,v 1.2 1990-11-21 14:25:51 epeisach Exp $

CFLAGS = -O

GCC= gcc
all: cleanup

cleanup: cleanup.o
	$(CC) -o cleanup cleanup.o -lhesiod

cleanup.o: cleanup.c
	$(GCC) $(CFLAGS) -c cleanup.c

clean:
	rm -f *.o cleanup

install: cleanup
	install -c -s cleanup /etc/athena/cleanup

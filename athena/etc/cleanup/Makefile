# $Header: /afs/dev.mit.edu/source/repository/athena/etc/cleanup/Makefile,v 1.1 1990-11-18 18:24:03 mar Exp $

CFLAGS = -O
GCC = /mit/gnu/@sys/gcc

all: cleanup

cleanup: cleanup.o
	$(CC) -o cleanup cleanup.o -lhesiod

cleanup.o: cleanup.c
	$(GCC) $(CFLAGS) -c cleanup.c

clean:
	rm -f *.o cleanup

install: cleanup
	install -c -s cleanup /etc/athena/cleanup

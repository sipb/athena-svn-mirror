# $Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/dm/Makefile,v 1.1 1990-10-18 18:58:21 mar Exp $

CFLAGS = -O -DDEBUG

CFILES = main.c
OBJS = main.o

all: dm

dm: $(OBJS)
	cc -o dm $(OBJS)

clean:
	rm -f *.o dm

depend: $(CFILES)
	mkdep $(CFLAGS) $(CFILES)


# DO NOT DELETE THIS LINE -- mkdep uses it.

main.o: main.c /usr/include/mit-copyright.h /usr/include/stdio.h
main.o: /usr/include/signal.h /usr/include/sys/ioctl.h
main.o: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
main.o: /usr/include/sys/wait.h /usr/include/sys/file.h /usr/include/utmp.h
main.o: /usr/include/ctype.h

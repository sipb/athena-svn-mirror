# $Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/dm/Makefile,v 1.3 1990-11-16 14:45:50 mar Exp $

CFLAGS = -O

CFILES = dm.c
OBJS = dm.o

all: dm

dm: $(OBJS)
	$(CC) -o dm $(OBJS)

clean:
	rm -f *.o dm

install: dm
	install -c -s dm $(DESTDIR)/etc/athena/dm
	install -c dm.8 $(DESTDIR)/usr/man/man8/dm.8

depend: $(CFILES)
	mkdep $(CFLAGS) $(CFILES)


# DO NOT DELETE THIS LINE -- mkdep uses it.

dm.o: dm.c /usr/include/mit-copyright.h /usr/include/signal.h
dm.o: /usr/include/sys/types.h /usr/include/sys/ioctl.h
dm.o: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
dm.o: /usr/include/sys/wait.h /usr/include/sys/file.h /usr/include/sys/stat.h
dm.o: /usr/include/sys/time.h /usr/include/sys/time.h /usr/include/utmp.h
dm.o: /usr/include/ctype.h /usr/include/strings.h

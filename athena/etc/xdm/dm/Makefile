# $Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/dm/Makefile,v 1.2 1990-10-22 19:42:39 mar Exp $

CFLAGS = -O -DDEBUG
CC = gcc

CFILES = dm.c config_console.c
OBJS = dm.o

all: dm config_console

dm: $(OBJS)
	$(CC) -o dm $(OBJS)

config_console: config_console.o
	$(CC) -o config_console config_console.o

clean:
	rm -f *.o dm config_console

install: dm
	install -c -s dm $(DESTDIR)/etc/athena/dm
	install -c -s config_console $(DESTDIR)/usr/athena
	ln -s /usr/athena/config_console $(DESTDIR)/usr/athena/show_console
	ln -s /usr/athena/condig_console $(DESTDIR)/usr/athena/hide_console


depend: $(CFILES)
	mkdep $(CFLAGS) $(CFILES)


# DO NOT DELETE THIS LINE -- mkdep uses it.

dm.o: dm.c /usr/include/mit-copyright.h /usr/include/signal.h
dm.o: /usr/include/sys/types.h /usr/include/sys/ioctl.h
dm.o: /usr/include/sys/ttychars.h /usr/include/sys/ttydev.h
dm.o: /usr/include/sys/wait.h /usr/include/sys/file.h /usr/include/sys/stat.h
dm.o: /usr/include/utmp.h /usr/include/ctype.h /usr/include/strings.h
config_console.o: config_console.c /usr/include/stdio.h /usr/include/signal.h
config_console.o: /usr/include/sys/file.h /usr/include/strings.h

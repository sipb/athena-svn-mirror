# $Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/console/Makefile,v 1.1 1990-11-16 15:12:33 mar Exp $

CFLAGS = -O

CFILES = config_console.c
OBJS = config_console.o

all: config_console

config_console: config_console.o
	$(CC) -o config_console config_console.o

clean:
	rm -f *.o config_console

install: config_console
	install -c -s -m 4755 -o root config_console $(DESTDIR)/usr/athena/config_console
	ln -s /usr/athena/config_console $(DESTDIR)/usr/athena/show_console
	ln -s /usr/athena/condig_console $(DESTDIR)/usr/athena/hide_console
	install -c config_console.1 $(DESTDIR)/usr/man/man1/config_console.1
	install -c hide_console.1 $(DESTDIR)/usr/man/man1/hide_console.1
	install -c show_console.1 $(DESTDIR)/usr/man/man1/show_console.1

depend: $(CFILES)
	mkdep $(CFLAGS) $(CFILES)


# DO NOT DELETE THIS LINE -- mkdep uses it.

config_console.o: config_console.c /usr/include/stdio.h /usr/include/signal.h
config_console.o: /usr/include/sys/file.h /usr/include/strings.h

#
# Makefile for 'xversion'
# Copyright 1989 by the Massachusetts Institute of Technology.
#
# $Header: /afs/dev.mit.edu/source/repository/athena/bin/xversion/Makefile,v 1.3 1990-02-09 09:00:59 epeisach Exp $
#

CFLAGS=	-O
LDFLAGS=-lX11

BINS=	xversion
SRCS=	xversion.c

all:	$(BINS)

clean:
	rm -f *.o *.a *~ core $(BINS)

install:
	-for i in $(BINS); do \
		install -c -s -m 0755 $$i $(DESTDIR)/usr/athena/; \
		done
	-install -c -m 0444 xversion.man $(DESTDIR)/usr/man/man1/xversion.1

depend:
	touch Make.depend; makedepend -fMake.depend -o "" -s "# DO NOT DELETE THIS LINE -- make depend uses it"\
		$(CFLAGS) $(SRCS)

$(BINS):
	$(CC) $(CFLAGS) -o $@ $@.c $(LDFLAGS)

xversion: xversion.c

# DO NOT DELETE THIS LINE -- make depend uses it

xversion: /usr/include/X11/copyright.h /usr/include/X11/Xlib.h
xversion: /usr/include/sys/types.h /usr/include/X11/X.h /usr/include/stdio.h
xversion: /usr/include/sys/socket.h /usr/include/sys/un.h

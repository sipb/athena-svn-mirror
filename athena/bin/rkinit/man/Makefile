#  
#  $Id: Makefile,v 1.2 1990-07-17 13:59:25 qjb Exp $
#  $Source: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/man/Makefile,v $
#  $Author: qjb $
#
# Makefile for rkinit manual pages
#  

DESTDIR		=
INSTALLDIR	= /usr/man

all:

depend:

install:
	install -c -m 444 rkinit.1 $(DESTDIR)$(INSTALLDIR)/man1/rkinit.1
	install -c -m 444 rkinitd.8 $(DESTDIR)$(INSTALLDIR)/man8/rkinitd.8
	install -c -m 444 rkinit.3 $(DESTDIR)$(INSTALLDIR)/man3/rkinit.3

clean:
	rm -f *~

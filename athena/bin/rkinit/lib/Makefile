#  
#  $Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/lib/Makefile,v 1.5 1990-07-17 12:33:53 qjb Exp $
#  $Source: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/lib/Makefile,v $
#  $Author: qjb $
#
#  This is the makefile for librkinit.a.
#

DESTDIR		=
ATHLIBDIR	= /usr/athena/lib

DFLAGS		= -O
INCLUDES	= -I../include
CFLAGS 		= $(DFLAGS) $(INCLUDES)

RKERR_H		= ../include/rkinit_err.h
RKERR_ET	= rkinit_err.et

LSRCS		= rk_lib.c rk_rpc.c rk_util.c rk_krb.c 
LOBJS		= rk_lib.o rk_rpc.o rk_util.o rk_krb.o rkinit_err.o
HEADERS		= ../include/rkinit.h ../include/rkinit_private.h $(RKERR_H)

all: librkinit.a

librkinit.a: $(LOBJS)
	-rm -f librkinit.a
	ar cru librkinit.a $(LOBJS)
	ranlib librkinit.a

$(LOBJS): $(HEADERS)

rkinit_err.c $(RKERR_H): $(RKERR_ET)
	compile_et rkinit_err.et
	mv rkinit_err.h ../include

depend:
	makedepend $(CFLAGS) $(LSRCS)

install:
	install -c -m 444 librkinit.a $(DESTDIR)$(ATHLIBDIR)/librkinit.a

clean:
	rm -f core *.o *~ *.bak librkinit.a rkinit_err.h rkinit_err.c

# DO NOT DELETE THIS LINE -- make depend depends on it.

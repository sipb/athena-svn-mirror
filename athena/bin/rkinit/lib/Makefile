#  
#  $Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/lib/Makefile,v 1.1 1989-11-13 15:05:32 qjb Exp $
#  $Source: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/lib/Makefile,v $
#  $Author: qjb $
#
#  This is the makefile for librkinit.a.
#

DESTDIR		=
INSTALLDIR	= /usr/athena/lib

KRB_TOP		= /afs/athena.mit.edu/system/krbuser/@sys
KLIBS		= -L$(KRB_TOP)/lib -lkrb -ldes

DFLAGS		= -O
INCLUDES	= -I$(KRB_TOP)/include -I../include
CFLAGS 		= $(DFLAGS) $(INCLUDES)
LIBS		= $(KLIBS) -lcom_err

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
	install -s librkinit.a $(DESTDIR)$(INSTALLDIR)/librkinit.a

clean:
	rm -f core *.o *~ librkinit.a rkinit_err.h rkinit_err.c

# DO NOT DELETE THIS LINE -- make depend depends on it.

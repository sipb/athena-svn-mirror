#  
#  $Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/rkinit/Makefile,v 1.3 1990-07-03 14:56:03 qjb Exp $
#  $Source: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/rkinit/Makefile,v $
#  $Author: qjb $
#
#  This is the makefile for rkinit.
#

DESTDIR		= 
INSTALLDIR	= /usr/athena

KLIBS		= -lkrb -ldes

RKLIB		= ../lib/librkinit.a

DFLAGS		= -O
INCLUDES	= -I../include
CFLAGS 		= $(DFLAGS) $(INCLUDES)
LIBS		= $(RKLIB) $(KLIBS) -lcom_err

RKERR_H		= ../include/rkinit_err.h
RKLIBDEP	= ../lib/rk_rpc.o ../lib/rk_lib.o ../lib/rk_util.o \
		  ../lib/rk_krb.o ../lib/rkinit_err.et

CSRCS		= rkinit.c
COBJS		= rkinit.o 
HEADERS		= ../include/rkinit.h $(RKERR_H)

all: rkinit

rkinit: $(COBJS) $(RKLIB)
	cc -o rkinit $(COBJS) $(LIBS) 

$(COBJS): $(HEADERS)

$(RKLIB): $(RKLIBDEP)

$(RKERR_H) $(RKLIBDEP):
	chdir ../lib; make 

depend:
	makedepend $(CFLAGS) $(CSRCS)

install:
	install -s rkinit $(DESTDIR)$(INSTALLDIR)/rkinit

clean:
	rm -f core *.o *~ *.bak rkinit

# DO NOT DELETE THIS LINE -- make depend depends on it.

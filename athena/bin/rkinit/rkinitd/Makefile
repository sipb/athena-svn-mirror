#  
#  $Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/rkinitd/Makefile,v 1.6 1990-08-09 15:33:00 qjb Exp $
#  $Source: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/rkinitd/Makefile,v $
#  $Author: qjb $
#
#  This is the makefile for rkinitd.
#

DESTDIR		=
INSTALLDIR	= /etc/athena

KLIBS		= -lkrb -ldes

RKLIB		= ../lib/librkinit.a

DFLAGS		= -O
INCLUDES	= -I../include
CFLAGS 		= $(DFLAGS) $(INCLUDES)
LIBS		= $(RKLIB) $(KLIBS) -lcom_err

RKERR_H		= ../include/rkinit_err.h
RKLIBDEP	= ../lib/rk_rpc.o ../lib/rk_lib.o ../lib/rk_util.o \
		  ../lib/rk_krb.o ../lib/rkinit_err.et

SSRCS		= rkinitd.c util.c rpc.c krb.c
SOBJS		= rkinitd.o util.o rpc.o krb.o
HEADERS		= ../include/rkinit.h ../include/rkinit_private.h $(RKERR_H)

all: rkinitd

rkinitd: $(SOBJS) $(RKLIB)
	$(CC) -o rkinitd $(SOBJS) $(LIBS)

$(SOBJS): $(HEADERS)

$(RKLIB): $(RKLIBDEP)

$(RKERR_H) $(RKLIBDEP): 
	cd ../lib; make 

depend:
	makedepend $(CFLAGS) $(SSRCS)

install:
	install -c -s rkinitd $(DESTDIR)$(INSTALLDIR)/rkinitd

clean:
	rm -f core *.o *~ *.bak rkinitd

# DO NOT DELETE THIS LINE -- make depend depends on it.

#
# Makefile for the FX (file exchange) error tables
#
# $Author: epeisach $
# $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/ets/Makefile,v $
# $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/ets/Makefile,v 1.1 1991-02-13 09:40:16 epeisach Exp $
#
# Copyright 1989, 1990 by the Massachusetts Institute of Technology.
#
# For copying and distribution information, please see the file
# <mit-copyright.h>.
#

.SUFFIXES: .et .o

CFLAGS = -O

.et.c:
	rm -f $*.o $*.c
	compile_et $*.et
	rm -f ../include/$*.h
	mv $*.h ../include

all: rpc_err.o fxserver_err.o fxcl_err.o krb_err.o

rpc_err.c: rpc_err.et
rpc_err.o: rpc_err.c
fxserver_err.c: fxserver_err.et
fxserver_err.o: fxserver_err.c
fxcl_err.c: fxcl_err.et
fxcl_err.o: fxcl_err.c
krb_err.c: krb_err.et
krb_err.o: krb_err.c

krb_err.et: mk_krb_err
	mk_krb_err > $@

mk_krb_err: mk_krb_err.c
	$(CC) ${CFLAGS} -o $@ mk_krb_err.c -lkrb

clean:
	rm -f core *.o *~ rpc_err.c fxserver_err.c fxcl_err.c \
	krb_err.c krb_err.et mk_krb_err

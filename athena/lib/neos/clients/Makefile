#
# Makefile for the FX (file exchange) clients
#
# $Author: epeisach $
# $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/Makefile,v $
# $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/clients/Makefile,v 1.1 1991-02-13 09:49:38 epeisach Exp $
#
# Copyright 1989, 1990 by the Massachusetts Institute of Technology.
#
# For copying and distribution information, please see the file
# <mit-copyright.h>.
#

CLIENTS = turnin pickup collect uncollect return fxpurge \
	 fxblanche fxcreate fxdestroy fxls fxping

INCLUDES = -I../lib -I../rpc3.9/usr/include -I../include
LIBS = -L../lib -L../rpc3.9/usr/lib -lss -lfxcl -lhesiod -lkrb -ldes -lcom_err -lrpclib
FXOBJS = main.o fx_ct.o fx.o arg.o
FXSRCS = main.c fx_ct.o fx.c arg.c

CFLAGS = -O ${INCLUDES}
INSTALL = install -c -s

.SUFFIXES: .o .h .lint .c .ct

.ct.c:
	rm -f $*.c
	mk_cmds $*.ct

.c.lint:
	lint -u $(INCLUDES) $*.c $(LIBS) | tee $@

all: $(CLIENTS)

install: all
	for i in $(CLIENTS); do \
	$(INSTALL) $$i $(DESTDIR)/usr/athena/$$i; done

turnin: turnin.o fxmain.o full_name.o
	$(CC) $(CFLAGS) -o turnin turnin.o fxmain.o full_name.o $(LIBS)

pickup: pickup.o dump.o array.o fxmain.o
	$(CC) $(CFLAGS) -o $@ dump.o pickup.o array.o fxmain.o $(LIBS)

collect: collect.o full_name.o dump.o array.o fxmain.o
	$(CC) $(CFLAGS) -o $@ full_name.o dump.o collect.o array.o fxmain.o $(LIBS)

uncollect: uncollect.o fxmain.o
	$(CC) $(CFLAGS) -o $@ uncollect.o fxmain.o $(LIBS)

return: return.o array.o full_name.o fxmain.o
	$(CC) $(CFLAGS) -o $@ return.o array.o full_name.o fxmain.o $(LIBS)

fxpurge: fxpurge.o full_name.o purge.o array.o fxmain.o
	$(CC) $(CFLAGS) -o $@ full_name.o purge.o fxpurge.o array.o fxmain.o $(LIBS)

fxcreate: fxcreate.o full_name.o
	$(CC) $(CFLAGS) -o fxcreate fxcreate.o full_name.o $(LIBS)

fxdestroy: fxdestroy.o
	$(CC) $(CFLAGS) -o fxdestroy fxdestroy.o $(LIBS)

fxblanche: fxblanche.o full_name.o
	$(CC) $(CFLAGS) -o $@ fxblanche.o full_name.o $(LIBS)

fxls: fxls.o
	$(CC) $(CFLAGS) -o $@ fxls.o $(LIBS)

fxping: fxping.o
	$(CC) $(CFLAGS) -o $@ fxping.o $(LIBS)

clean:
	rm -f *.lint *.o core $(CLIENTS)

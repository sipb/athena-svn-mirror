#
# @(#)Makefile	1.6 87/11/30 3.9 RPCSRC
#
#   Build and install everything.
#
# These directories are presumed to exist in DESTDIR:
#    /usr/lib /usr/bin /usr/include
#
# Hacked by R. French 7/25/89 to fit into the File Exchange system.
#

LIB=-lrpclib
DESTDIR=..
CFLAGS=-O -L../usr/lib -I../usr/include -I..
MAKE=make

SUBDIR= rpcgen rpc

all:	${SUBDIR}

rpclib:	FRC
	@echo "Building and installing RPC library"
	cd rpc; $(MAKE) ${MFLAGS} all DESTDIR=${DESTDIR} LIB=$(LIB); \
        $(MAKE) ${MFLAGS} install DESTDIR=${DESTDIR}

${SUBDIR}:	FRC
	@echo "Building and installing files from: $@"
	cd $@; $(MAKE) ${MFLAGS} DESTDIR=${DESTDIR} "CFLAGS=${CFLAGS}" LIB=$(LIB);\
        $(MAKE) ${MFLAGS} install DESTDIR=${DESTDIR} LIB=$(LIB)

FRC:

clean:
	(cd rpcgen; make clean)
	(cd rpc; make clean)
	rm -rf ./usr/include/rpc
	rm -f ./usr/bin/rpcgen
	rm -f ./usr/lib/librpclib.a

# Makefile for /usr/ucb (4.3 standard)
#
#	MIT Project Athena
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/etc/inetd/Makefile,v $
#	$Author: vrt $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/etc/inetd/Makefile,v 1.1 1994-04-02 13:25:01 vrt Exp $
#
# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.17 (Berkeley) 6/7/86
#
DESTDIR=
CFLAGS=	-O -DPOSIX -DSYSV -DSOLARIS -lsocket -lnsl


# C programs that live in the current directory and do not need
# explicit make lines.
STD=inetd

all:	${STD} 

${STD} ${KMEM} ${SETUID} ${OPR}:
	cc ${CFLAGS} -o $@ $@.c


install:
	-for i in ${STD} ; do \
		rm ${DESTDIR}/etc/$$i; \
		install -c -s $$i ${DESTDIR}/etc/$$i; done

clean:
	rm -f a.out core *.s *.o *~
	rm -f ${STD}




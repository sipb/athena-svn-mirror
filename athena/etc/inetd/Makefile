# Makefile for /usr/ucb (4.3 standard)
#
#	MIT Project Athena
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/etc/inetd/Makefile,v $
#	$Author: miki $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/etc/inetd/Makefile,v 1.3 1994-05-04 11:41:47 miki Exp $
#
# Copyright (c) 1980 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.17 (Berkeley) 6/7/86
#
DESTDIR=
CFLAGS=	-O -DPOSIX -DSYSV -DSOLARIS 
LDFLAGS= /usr/ucblib/libucb.a -lsocket -lnsl


# C programs that live in the current directory and do not need
# explicit make lines.
STD=inetd

all:	${STD} 

${STD} ${KMEM} ${SETUID} ${OPR}:
	cc ${CFLAGS} -o $@ $@.c ${LDFLAGS}


install: installman
	-for i in ${STD} ; do \
		rm ${DESTDIR}/etc/$$i; \
		install -c -s $$i ${DESTDIR}/etc/$$i; done
       
installman:	
	install -c inetd.conf.5  ${DESTDIR}/usr/athena/man/man5/inetd.conf.5
	install -c inetd.8  ${DESTDIR}/usr/athena/man/man5/inetd.8       
clean:
	rm -f a.out core *.s *.o *~
	rm -f ${STD}

depend::




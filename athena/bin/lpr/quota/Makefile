#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/Makefile,v $
#	$Author: epeisach $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/Makefile,v 1.1 1990-04-16 16:26:31 epeisach Exp $
#
#
# Copyright (c) 1983 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.3 (Berkeley) 5/7/86
#
# makefile for line printer spooling system
#
# Ownerships {see note at the top of lpd.c}
# ROOT		a user that can access any and all files
# DAEMON	someone special
# SPGRP		the group id of the spooling programs
#
DESTDIR=

CFLAGS=-O -DHESIOD -DKERBEROS -DZEPHYR -DNDBM -I./ncs/include/idl -I. -I..
LIBS= ./ncs/${MACHINE}lib/libnck.a -lhesiod -lzephyr -lacl -lkrb -ldes -lcom_err
NIDL=./ncs/${MACHINE}bin/nidl
NIDLOPTS=-idir ./ncs/include/idl -m
.SUFFIXES: .idl .h
CPP=./ncs/${MACHINE}bin/cpp

.c.o:
	 ${CPP} $(CFLAGS) $*.c x.c; $(CC) -c $(CFLAGS) x.c; mv x.o $*.o

.idl.h:
	$(NIDL) $*.idl $(NIDLOPTS)



SRCS=	qmain.c quota_dba.c quota_ncs_sstub.c quota_server.c quotacap.c \
	server_krb.c quota_notify.c quota_logger.c strings.c parser.c \
	journal.c user.c periodic.c logger_ncs_sstub.c logger_server.c


OBJS=	qmain.o quota_dba.o quota_ncs_sstub.o quota_server.o quotacap.o \
	server_krb.o quota_notify.o quota_logger.o strings.o parser.o \
	journal.o user.o periodic.o logger_ncs_sstub.o logger_server.o

CSRCS=	lpquota.c quota_ncs_cstub.c quota_ncs_cswtch.c quota_err.c \
	logger_ncs_cstub.c logger_ncs_cswtch.c printcap.c
COBJS=	lpquota.o quota_ncs_cstub.o quota_ncs_cswtch.o quota_err.o \
	logger_ncs_cstub.o logger_ncs_cswtch.o printcap.o

LSRCS= quota_dba.c qdb_create.c
LOBJS= quota_dba.o qdb_create.o

SSRCS= s_chkaf.c quota_ncs_cstub.c quota_ncs_cswtch.c printcap.c 

SOBJS= s_chkaf.o quota_ncs_cstub.o quota_ncs_cswtch.o printcap.o

ALL=	lpqd lpquota qdb_create s_chkaf

all:	${ALL} 

s_chkaf: ${SOBJS}
	${CC} -o s_chkaf ${SOBJS} ${LIBS}

lpqd:	${OBJS}
	cc -o lpqd ${OBJS} ${LIBS}

lint_lpqd: ${SRCS}
	lint -u ${CFLAGS}  ${SRCS}

saber_lpqd:
	#load ${CFLAGS} ${SRCS} ${LIBS}

saber_lpquota:
	#load ${CFLAGS} ${CSRCS} ${LIBS}

lpquota: ${COBJS}
	cc -o lpquota ${COBJS} ${LIBS}

lint_lpquota:
	lint -u ${CFLAGS} lpquota.c 

lint_s_chkaf:
	lint -u ${CFLAGS} s_chkaf.c

qdb_create: ${LOBJS}
	cc -o qdb_create ${LOBJS} ${LIBS}
clean:
	rm -f quota_ncs_sstub.c quota_ncs_cswtch.c quota_ncs_cstub.c \
		logger_ncs_sstub.c logger_ncs_cswtch.c logger_ncs_cstub.c \
		logger_ncs.h quota_ncs.h quota_err.[ch] *.o *~ ${ALL} \
		x.c printcap.c

depend: ${SRCS} ${CSRCS} ${LSRCS}
	touch Make.depend; mkdep -fMake.depend ${CFLAGS} ${SRCS} \
	${CSRCS} ${LSRCS}

printcap.c:
	rm -f printcap.c
	ln -s ../printcap.c 

quota_ncs_sstub.c: quota_ncs.h
quota_ncs_cstub.c: quota_ncs.h
quota_ncs_cswtch.c: quota_ncs.h
quota_ncs.h: quota_ncs.idl

logger_ncs_sstub.c: logger_ncs.h
logger_ncs_cstub.c: logger_ncs.h
logger_ncs_cswtch.c: logger_ncs.h
logger_ncs.h: logger_ncs.idl logger.h

quota_err.h quota_err.c:quota_err.et
	compile_et quota_err.et

qmain.o: quota_ncs.h logger_ncs.h
quota_server.o: quota_err.h
parser.o: logger_ncs.h

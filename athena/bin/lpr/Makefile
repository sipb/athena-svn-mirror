#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/Makefile,v $
#	$Author: shanzer $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/Makefile,v 1.2 1989-05-31 11:45:11 shanzer Exp $
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

CFLAGS=-O -DVFS -DHESIOD
LIBS= -lhesiod
LIBDIR=/usr/lib
BINDIR=/usr/ucb
SPOOLDIR=/usr/spool/lpd
CXREF=/usr/ucb/ctags -x
ROOT=root
DAEMON=daemon
SPGRP=daemon
OPERATOR=OPERATOR
# OP_GID is the group ID for group operator
OP_GID = 28
SRCS=	lpd.c lpr.c lpq.c lprm.c pac.c lpd.c cmds.c cmdtab.c \
	printjob.c recvjob.c displayq.c rmjob.c \
	startdaemon.c common.c printcap.c lpdchar.c
ALL=	lpd lpc lptest pac o_lprm o_lpc lpr lpq lprm s_lpq s_lprm s_lpr

all:	${ALL} FILTERS

lpd:	lpd.o printjob.o recvjob.o s_displayq.o s_rmjob.o 
lpd:	lpdchar.o s_common.o printcap.o 
	${CC} -o lpd lpd.o printjob.o recvjob.o s_displayq.o s_rmjob.o \
		lpdchar.o s_common.o printcap.o ${LIBS}

s_rmjob.o: rmjob.c lp.h lp.local.h
	rm -f s_rmjob.c
	ln rmjob.c s_rmjob.c
	${CC} ${CFLAGS} -c -DSERVER s_rmjob.c

s_common.o: lp.h lp.local.h common.c
	rm -f s_common.c
	ln common.c s_common.c
	${CC} ${CFLAGS} -c -DSERVER s_common.c

s_lpr.o: lpr.c lp.h lp.local.h
	rm -f s_lpr.c
	ln lpr.c s_lpr.c
	$(CC) ${CFLAGS} -c -DSERVER s_lpr.c

s_lpq.o: lpq.c lp.h lp.local.h
	rm -f s_lpq.c
	ln lpq.c s_lpq.c
	$(CC) ${CFLAGS} -c -DSERVER s_lpq.c

s_lprm.o: lprm.c lp.h lp.local.h
	rm -f s_lprm.c
	ln lprm.c s_lprm.c
	$(CC) ${CFLAGS} -c -DSERVER s_lprm.c

lpd.o: lpd.c
	${CC} -c ${CFLAGS} -Dws lpd.c

lpr:	lpr.o printcap.o netsend.o common.o
	${CC} -o lpr lpr.o printcap.o common.o netsend.o ${LIBS}

s_lpr:	s_lpr.o startdaemon.o printcap.o 
	${CC} -o s_lpr s_lpr.o startdaemon.o printcap.o ${LIBS}

lpq:	lpq.o displayq.o common.o printcap.o
	${CC} -o lpq lpq.o displayq.o common.o printcap.o -ltermcap ${LIBS}

s_lpq:	s_lpq.o s_displayq.o s_common.o printcap.o startdaemon.o
	${CC} -o s_lpq s_lpq.o s_displayq.o s_common.o startdaemon.o \
		printcap.o -ltermcap ${LIBS}

lprm:	lprm.o rmjob.o common.o printcap.o 
	${CC} -o lprm lprm.o rmjob.o common.o printcap.o ${LIBS}

s_lprm:	s_lprm.o s_rmjob.o startdaemon.o s_common.o printcap.o 
	${CC} -o s_lprm s_lprm.o s_rmjob.o startdaemon.o s_common.o \
		printcap.o ${LIBS}

o_lprm:	o_lprm.o s_rmjob.o startdaemon.o s_common.o printcap.o 
	${CC} -o o_lprm o_lprm.o s_rmjob.o startdaemon.o s_common.o \
		printcap.o ${LIBS}

lpc:	lpc.o cmds.o cmdtab.o startdaemon.o s_common.o printcap.o 
	${CC} -o lpc lpc.o cmds.o cmdtab.o startdaemon.o s_common.o \
		printcap.o ${LIBS}

o_lpc:	o_lpc.o cmds.o cmdtab.o startdaemon.o s_common.o printcap.o 
	${CC} -o o_lpc o_lpc.o cmds.o cmdtab.o startdaemon.o s_common.o \
		 printcap.o ${LIBS}

lptest:	lptest.c
	${CC} ${CFLAGS} -o lptest lptest.c

pac:	pac.o printcap.o
	${CC} -o pac pac.o printcap.o ${LIBS}

o_lprm.o: lp.h lp.local.h 
	rm -f o_lprm.c 
	ln lprm.c o_lprm.c
	${CC} ${CFLAGS} -c -D${OPERATOR} o_lprm.c

o_lpc.o: lp.h lp.local.h 
	rm -f o_lpc.c
	ln lpc.c o_lpc.c
	${CC} ${CFLAGS} -c -D${OPERATOR} o_lpc.c

lpd.o lpr.o lpq.o lprm.o o_lprm.o pac.o: lp.h lp.local.h
recvjob.o printjob.o displayq.o rmjob.o common.o: lp.h lp.local.h
startdaemon.o: lp.local.h
lpc.o o_lpc.o cmdtab.o: lpc.h
cmds.o: lp.h lp.local.h

FILTERS:
	cd filters; make ${MFLAGS}

install: all
#	install -c -m 444 printcap ${DESTDIR}/etc/printcap
	install -c -s -g ${SPGRP} -m 6755 lpd ${DESTDIR}/${LIBDIR}/lpd
	install -c -s -g ${SPGRP} -m 6755 lpr ${DESTDIR}/${BINDIR}/lpr
	install -c -s -g ${SPGRP} -m 6755 lpq ${DESTDIR}/${BINDIR}/lpq
	install -c -s -g ${SPGRP} -m 6755 lprm ${DESTDIR}/${BINDIR}/lprm
	install -c -s -g ${SPGRP} -m 6755 s_lpr ${DESTDIR}/${BINDIR}/s_lpr
	install -c -s -g ${SPGRP} -m 6755 s_lpq ${DESTDIR}/${BINDIR}/s_lpq
	install -c -s -g ${SPGRP} -m 6755 s_lprm ${DESTDIR}/${BINDIR}/s_lprm
	install -c -s -g ${OP_GID} -m 6754 o_lprm ${DESTDIR}/usr/etc/o_lprm
	install -c -s -g ${SPGRP} -m 2755 lpc ${DESTDIR}/usr/etc/lpc
	install -c -s -g ${OP_GID} -m 6754 o_lpc ${DESTDIR}/usr/etc/o_lpc
#	install -s lptest ${DESTDIR}/${BINDIR}/lptest
	install -s pac ${DESTDIR}/usr/etc/pac
	install -c print.sh ${DESTDIR}/usr/ucb/print
	install -c -m 755 makespools ${DESTDIR}/etc

	@echo  To build spooling directories:
	@echo makespools ${DESTDIR} 775 ${DAEMON} ${SPGRP}

#	chown ${DAEMON} ${DESTDIR}/${SPOOLDIR}
#	chgrp ${SPGRP} ${DESTDIR}/${SPOOLDIR}
#	chmod 775 ${DESTDIR}/${SPOOLDIR}
	cd filters; make ${MFLAGS} DESTDIR=${DESTDIR} install

clean:
	rm -f ${ALL} *.o *~
	cd filters; make ${MFLAGS} clean

print:
	@pr makefile
	@${CXREF} *.c | pr -h XREF
	@pr *.h *.c

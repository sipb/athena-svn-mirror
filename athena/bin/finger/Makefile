# Makefile for Hesiod/Zephyr client finger
#
#	MIT Project Athena
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/bin/finger/Makefile,v $
#	$Header: /afs/dev.mit.edu/source/repository/athena/bin/finger/Makefile,v 1.8 1989-10-18 13:21:16 epeisach Exp $
#	$Author: epeisach $
#	$Log: not supported by cvs2svn $
# Revision 1.7  89/02/22  16:51:28  epeisach
# Install options to strip and set permissions added.
# 
# Revision 1.6  87/12/04  11:36:25  shanzer
# Added -ldes to the list of libriaries..
# 
# Revision 1.5  87/08/27  16:56:33  ambar
# fixed clean target.
# 
# Revision 1.5  87/08/27  16:41:44  ambar
# fixed clean target
# 
# Revision 1.4  87/08/21  18:30:58  ambar
# typo fix
# 
# Revision 1.3  87/08/21  18:28:18  ambar
# changed the name of the hesiod file.
# 
# Revision 1.2  87/08/20  16:11:21  ambar
# 
# oops: excess backslash removed.
# 
# Revision 1.1  87/08/20  16:02:51  ambar
# Initial revision
# 
#
DESTDIR=
CFLAGS = -O
CONFDIR = ${DESTDIR}/usr/athena
BINDIR = ${DESTDIR}/bin

LIBS =  -lhesiod -lzephyr -lcom_err -lkrb -ldes

SRCS =	finger.c hespwnam.c

OBJECTS = finger.o hespwnam.o

all:	finger

finger:	${OBJECTS}
	cc ${CFLAGS} -o finger ${OBJECTS} ${LIBS}

finger.o:	finger.c
	cc -c ${CFLAGS} finger.c

hespwnam.o:	hespwnam.c
	cc -c ${CFLAGS} hespwnam.c

lint:
	lint -I../../include *.c

clean:	
	rm -f *.o *~ *.BAK
	rm -f finger

install:	finger
	install -c -s -m 755 finger ${DESTDIR}/usr/athena/finger

depend:
	makedepend ${CFLAGS} ${SRCS}

# DO NOT DELETE THIS LINE -- make depend depends on it.

finger.o: finger.c /usr/include/sys/file.h /usr/include/sys/types.h
finger.o: /usr/include/sys/stat.h /usr/include/utmp.h
finger.o: /usr/include/sys/signal.h /usr/include/pwd.h /usr/include/stdio.h
finger.o: /usr/include/lastlog.h /usr/include/ctype.h /usr/include/sys/time.h
finger.o: /usr/include/sys/time.h /usr/include/sys/socket.h
finger.o: /usr/include/netinet/in.h /usr/include/netdb.h
finger.o: /usr/include/hesiod.h /usr/include/zephyr/zephyr.h
finger.o: /usr/include/zephyr/mit-copyright.h
finger.o: /usr/include/zephyr/zephyr_err.h /usr/include/zephyr/zephyr_conf.h
finger.o: /usr/include/errno.h /usr/include/krb.h
finger.o: /usr/include/mit-copyright.h /usr/include/des.h
finger.o: /usr/include/des_conf.h
hespwnam.o: hespwnam.c /usr/include/stdio.h /usr/include/pwd.h
hespwnam.o: /usr/include/mit-copyright.h

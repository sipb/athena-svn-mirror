#
# Copyright (c) 1988 Regents of the University of California.
# All rights reserved.
#
# Redistribution and use in source and binary forms are permitted
# provided that the above copyright notice and this paragraph are
# duplicated in all such forms and that any documentation,
# advertising materials, and other materials related to such
# distribution and use acknowledge that the software was developed
# by the University of California, Berkeley.  The name of the
# University may not be used to endorse or promote products derived
# from this software without specific prior written permission.
# THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
#	@(#)Makefile	5.8 (Berkeley) 9/22/88 + portability hacks by 
#						rick@seismo.css.gov
#
CFLAGS=	-O
LIBC=	/lib/libc.a
SRCS=	ftpd.c ftpcmd.c glob.c logwtmp.c popen.c vers.c
OBJS=	ftpd.o ftpcmd.o glob.o logwtmp.o popen.o vers.o strpbrk.o strtok.o \
	getusershell.o

all: ftpd

ftpd: ${OBJS} ${LIBC}
	${CC} -o $@ ${OBJS}

vers.o: ftpd.c ftpcmd.y
	sh newvers.sh
	${CC} ${CFLAGS} -c vers.c

clean:
	rm -f ${OBJS} ftpd core ftpcmd.c

cleandir: clean
	rm -f tags .depend

depend: ${SRCS}
	mkdep ${CFLAGS} ${SRCS}

install: 
	if test ! -s ${DESTDIR}/etc/shells ; then cp etc.shells /etc/shells; fi
	if test -s ${DESTDIR}/etc/ftpd; then \
		install -s -o bin -g bin -m 755 ftpd ${DESTDIR}/etc/ftpd; \
	else \
		install -s -o bin -g bin -m 755 ftpd ${DESTDIR}/usr/etc/in.ftpd; \
	fi

lint: ${SRCS}
	lint ${CFLAGS} ${SRCS}

tags: ${SRCS}
	ctags ${SRCS}

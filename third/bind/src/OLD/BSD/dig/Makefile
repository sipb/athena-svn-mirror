# $Id: Makefile,v 1.1.1.1 1998-05-04 22:23:32 ghudson Exp $

.PATH:	${.CURDIR}/../BIND/tools \
	${.CURDIR}/../BIND/tools/nslookup \
	${.CURDIR}/../BIND/man

PROG=	dig
SRCS=	dig.c list.c subr.c debug.c send.c

BINDIR=	/usr/bin

.include <bsd.prog.mk>
.include "${.CURDIR}/../Makefile.maninc"

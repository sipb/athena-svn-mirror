# Makefile for "track" automatic update program
#
# 	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/Makefile,v $
#	$Author: epeisach $
#	$Locker:  $
#	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/Makefile,v 1.12 1990-02-09 09:31:47 epeisach Exp $
DESTDIR=
INCDIR= /usr/include
CFLAGS=	-O -I${INCDIR}
LIBS= -ll

PROGS= track # nullmail

TRACK_OBJS= track.o y.tab.o stamp.o except.o files.o misc.o update.o cksum.o

TRACK_SRCS= track.c y.tab.c stamp.c except.c files.c misc.c update.c cksum.c

TRACK_DEP= track.h track.c stamp.c except.c files.c misc.c update.c nullmail.c

all: $(PROGS)

track: $(TRACK_OBJS)
	$(CC) -o track $(TRACK_OBJS) $(LIBS)

y.tab.o : y.tab.c lex.yy.c track.h
	cc -c y.tab.c

y.tab.c : sub_gram.y 
	yacc sub_gram.y

lex.yy.c : input.l
	@echo IGNORE FIVE "Non-Portable Character Class" WARNINGS --
	lex input.l

nullmail : nullmail.c
	$(CC) $(CFLAGS) -o nullmail nullmail.c
	
install:
	for i in $(PROGS); do \
		(install -c -s $$i $(DESTDIR)/etc/athena/$$i); \
	done
	(cd doc; make install ${MFLAGS} DESTDIR=${DESTDIR})

clean:
	/bin/rm -f a.out core *.o *~ y.tab.c lex.yy.c $(PROGS)

lint:
	lint -uahv $(TRACK_SRCS)

depend: $(TRACK_SRCS)
	touch Make.depend; makedepend -fMake.depend $(CFLAGS) ${TRACK_SRCS}

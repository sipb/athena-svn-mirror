# Makefile for "track" automatic update program
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/etc/track/Makefile,v $
#	$Header: /afs/dev.mit.edu/source/repository/athena/etc/track/Makefile,v 1.1 1987-02-12 21:12:35 rfrench Exp $
#
#	$Log: not supported by cvs2svn $

DESTDIR=
CONFDIR=../bin
CFLAGS= -g
LIBS= -ll

PROGS= track nullmail

TRACK_OBJS= track.o y.tab.o stamp.o except.o files.o misc.o update.o

TRACK_SRCS= track.c y.tab.c stamp.c except.c files.c misc.c update.c

all: $(PROGS)

track: $(TRACK_OBJS)
	$(CC) -g -o track $(TRACK_OBJS) $(LIBS)

y.tab.o : y.tab.c lex.yy.c track.h
	cc -c -g y.tab.c

y.tab.c : sub_gram.y 
	yacc sub_gram.y

lex.yy.c : input.l
	@echo IGNORE THE WARNINGS --
	lex input.l

nullmail : nullmail.c
	$(CC) $(CFLAGS) -o nullmail nullmail.c
	
install:
	for i in $(PROGS); do \
		(install -c -s $$i $(DESTDIR)$(CONFDIR)/$$i); \
	done

clean:
	/bin/rm -f *.o *~ y.tab.c lex.yy.c $(PROGS)

lint:
	lint -uahv $(TRACK_SRCS)

#     Copyright 1988 Massachusetts Institute of Technology.
#
#     For copying and distribution information, see the file
#     "mit-copyright.h".
#
#     $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/Makefile,v $
#     $Author: jik $
#     $Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/Makefile,v 1.5 1989-01-27 10:46:41 jik Exp $
#

TARGETS = delete undelete expunge purge lsdel
DESTDIR = /bin/athena
CC = cc
CFLAGS = -O
SRCS = delete.c undelete.c directories.c pattern.c util.c expunge.c \
	lsdel.c col.c

all: $(TARGETS)

install:
	for i in $(TARGETS)
	do
	cp $i $(DESTDIR)
	strip $(DESDTIR)/$i
	done

delete: delete.o util.o
	cc $(CFLAGS) -o delete delete.o util.o

undelete: undelete.o directories.o util.o pattern.o
	cc $(CFLAGS) -o undelete undelete.o directories.o util.o pattern.o

saber_undelete:
	#alias s step
	#alias n next
	#load undelete.c directories.c util.c pattern.c

expunge: expunge.o directories.o pattern.o util.o
	cc $(CFLAGS) -o expunge expunge.o directories.o pattern.o util.o

saber_expunge:
	#alias s step
	#alias n next
	#load expunge.c directories.c pattern.c util.c

purge: expunge
	ln -s expunge purge

lsdel: lsdel.o util.o directories.o pattern.o col.o
	cc $(CFLAGS) -o lsdel lsdel.o util.o directories.o pattern.o col.o

saber_lsdel:
	#alias s step
	#alias n next
	#load lsdel.c util.c directories.c pattern.c col.c

clean:
	-rm -f *~ *.bak *.o delete undelete lsdel expunge purge

depend: delete.c undelete.c
	/usr/athena/makedepend -v $(CFLAGS) -s'# DO NOT DELETE' $(SRCS)

# DO NOT DELETE THIS LINE -- makedepend depends on it

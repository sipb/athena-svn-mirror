# makefile for cxref -- C cross referencing program
#
# Arnold Robbins, Information and Computer Science, Georgia Tech
#	gatech!arnold
# Copyright (c) 1985 by Arnold Robbins.
# All rights reserved.
# This program may not be sold, but may be distributed
# provided this header is included.

# some files are system dependant, e.g. where sort is.
# change the appropriate macro definitions and recompile.


### definitions of files to compile and load, and other targets for make

SCANOBJS= docxref.o cscan.o
SCANSRCS= docxref.c cscan.l

CXREF = cxref
INCLS= constdefs.h basename.c
PROGS= docxref fmtxref cxrfilt $(CXREF)
SRCS=  $(SCANSRCS) fmtxref.c cxrfilt.c $(CXREF).c
DOCS=  README makefile cxref.1

PRINTS= $(INCLS) $(SRCS) $(DOCS)

CFLAGS= -O

### system dependant definitions, change when you install cxref

# for my use during development, put in my bin, but see next few lines.
DESTDIR=
CONFDIR=/usr/athena
LIB= /usr/athena/lib/cxref

# where to put the man page, use 1 instead of l if you don't have a manl.
MANSEC=1

# lex library, may be -lln on some systems
LEXLIB= -ll

# may be /bin/sort on some systems
SORT=/usr/bin/sort

# printer program, prt is for me, use pr on other systems
P=pr

# the owner and group of the installed program.  Both are 'admin' on our
# system, but they may different on yours.
OWNER= root
GROUP= staff

all: $(PROGS)
	@echo "	all" done

docxref: $(SCANOBJS)
	$(CC) $(SCANOBJS) $(LEXLIB) -o $@

cscan.o docxref.o cxrfilt.o: $(INCLS)

fmtxref: fmtxref.c
	$(CC) $(CFLAGS) $@.c $(LDFLAGS) -o $@

cxrfilt: cxrfilt.c
	$(CC) $(CFLAGS) $@.c $(LDFLAGS) -o $@

$(CXREF): $(CXREF).c
	$(CC) $(CFLAGS) -DSRCDIR='"$(LIB)"' -DSORT='"$(SORT)"' $@.c $(LDFLAGS) -o $@

print:
	$(P) $(PRINTS) | lpr -b 'Cxref Source'
	touch print2

print2: $(PRINTS)
	$(P) $? | lpr -b 'Cxref New Source'
	touch print2

install: $(PROGS)
	rm -fr $(DESTDIR)$(LIB)
	rm -f $(DESTDIR)$(CONFDIR)/cxref
	mkdir $(DESTDIR)$(LIB)
	install -s -m 755 -o $(OWNER) -g $(GROUP) $(CXREF) \
		$(DESTDIR)$(CONFDIR)/$(CXREF)
	for i in docxref fmtxref cxrfilt; do \
		install -s -m 755 -o $(OWNER) -g $(GROUP) $$i \
		$(DESTDIR)$(LIB)/$$i; \
	done

	install -m 644 cxref.1 $(DESTDIR)/usr/man/man$(MANSEC)/cxref.$(MANSEC)

clean:
	rm -f $(SCANOBJS)
	rm -f $(PROGS) print2


depend:
	touch Make.depend; makedepend -fMake.depend ${CFLAGS} ${SRCS}

# DO NOT DELETE THIS LINE -- make depend depends on it.

docxref.o: docxref.c /usr/include/stdio.h /usr/include/ctype.h basename.c
cscan.o: cscan.l constdefs.h
fmtxref.o: fmtxref.c /usr/include/stdio.h /usr/include/ctype.h basename.c
cxrfilt.o: cxrfilt.c /usr/include/stdio.h constdefs.h basename.c
cxref.o: cxref.c /usr/include/stdio.h /usr/include/ctype.h
cxref.o: /usr/include/signal.h /usr/include/sys/types.h
cxref.o: /usr/include/sys/stat.h basename.c

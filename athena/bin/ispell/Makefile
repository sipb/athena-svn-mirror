# -*- Mode: Text -*-

# Look over config.X before building.
#
# You may want to edit BINDIR, LIBDIR, DEFHASH, DEFDICT, MANDIR, and
# MANEXT below; the Makefile will update all other files to match.
#
# On USG systems, add -DUSG to CFLAGS.
#
# The ifdef NO8BIT may be used if 8 bit extended text characters
# cause problems, or you simply don't wish to allow the feature.
#
# the argument syntax for buildhash to make alternate dictionary files
# is simply:
#
#   buildhash <infile> <outfile>

CFLAGS = -O
# BINDIR, LIBDIR, DEFHASH, DEFDICT, MANDIR, MANEXT
BINDIR = /usr/athena
LIBDIR = /usr/athena/lib/ispell
DEFHASH = ispell.hash
DEFDICT = dict.191
MANDIR	= /usr/man/man1
MANEXT	= .1
SHELL = /bin/sh

TERMLIB = -ltermcap

all: buildhash ispell icombine munchlist $(DEFHASH)

ispell.hash: buildhash $(DEFDICT)
	./buildhash $(DEFDICT) $(DEFHASH)

install: all
	install -m 755 -s ispell $(BINDIR)/ispell
	install -m 755 -s icombine $(LIBDIR)/icombine
	install -m 755 munchlist $(BINDIR)/munchlist
	install -m 644 ispell.hash $(LIBDIR)/$(DEFHASH)
	install -m 644 expand1.sed $(LIBDIR)/expand1.sed
	install -m 644 expand2.sed $(LIBDIR)/expand2.sed
	install -m 644 ispell.1 $(MANDIR)/ispell.1

buildhash: buildhash.o hash.o
	$(CC) $(CFLAGS) -o buildhash buildhash.o hash.o

icombine:	icombine.c
	$(CC) $(CFLAGS) -o icombine icombine.c

munchlist:	munchlist.X Makefile
	sed -e 's@!!LIBDIR!!@$(LIBDIR)@' -e 's@!!DEFDICT!!@$(DEFDICT)@' \
		<munchlist.X >munchlist
	chmod +x munchlist

OBJS=ispell.o term.o good.o lookup.o hash.o tree.o
ispell: $(OBJS)
	cc $(CFLAGS) -o ispell $(OBJS) $(TERMLIB)

$(OBJS) buildhash.o: config.h

config.h:	config.X Makefile
	sed -e 's@!!LIBDIR!!@$(LIBDIR)@' -e 's@!!DEFDICT!!@$(DEFDICT)@' \
	    -e 's@!!DEFHASH!!@$(DEFHASH)@' <config.X >config.h

clean:
	rm -f *.o buildhash ispell core a.out mon.out hash.out \
		*.stat *.cnt munchlist config.h icombine

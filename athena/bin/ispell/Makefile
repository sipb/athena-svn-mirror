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

DESTDIR=
CFLAGS = -O
# BINDIR, LIBDIR, DEFHASH, DEFDICT, MANDIR, MANEXT
BINDIR = ${DESTDIR}/usr/athena
LIBDIR = ${DESTDIR}/usr/athena/lib/ispell
DEFHASH = ispell.hash
DEFDICT = dict.191
MANDIR	= ${DESTDIR}/usr/man/man1
MANEXT	= .1
SHELL = /bin/sh

TERMLIB = -ltermcap

all: buildhash ispell icombine munchlist $(DEFHASH)

ispell.hash: buildhash $(DEFDICT)
	./buildhash $(DEFDICT) $(DEFHASH)

install: all
	-mkdir ${BINDIR}
	-mkdir ${LIBDIR}
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
SRCS=ispell.c term.c good.c lookup.c hash.c tree.c
ispell: $(OBJS)
	cc $(CFLAGS) -o ispell $(OBJS) $(TERMLIB)

$(OBJS) buildhash.o: config.h

config.h:	config.X Makefile
	sed -e 's@!!LIBDIR!!@$(LIBDIR)@' -e 's@!!DEFDICT!!@$(DEFDICT)@' \
	    -e 's@!!DEFHASH!!@$(DEFHASH)@' <config.X >config.h

clean:
	rm -f *.o buildhash ispell core a.out mon.out hash.out \
		*.stat *.cnt munchlist config.h icombine

depend:
	makedepend ${CFLAGS} ${SRCS}

	
# DO NOT DELETE THIS LINE -- make depend depends on it.

ispell.o: ispell.c /usr/include/stdio.h /usr/include/ctype.h
ispell.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
ispell.o: /usr/include/sys/signal.h /usr/include/sys/types.h ispell.h
ispell.o: config.h
term.o: term.c /usr/include/stdio.h /usr/include/sgtty.h
term.o: /usr/include/sys/ioctl.h /usr/include/sys/ttychars.h
term.o: /usr/include/sys/ttydev.h /usr/include/sys/signal.h ispell.h
good.o: good.c /usr/include/stdio.h /usr/include/ctype.h ispell.h config.h
lookup.o: lookup.c /usr/include/stdio.h ispell.h config.h
hash.o: hash.c
tree.o: tree.c /usr/include/stdio.h /usr/include/ctype.h
tree.o: /usr/include/sys/param.h /usr/include/machine/machparam.h
tree.o: /usr/include/sys/signal.h /usr/include/sys/types.h ispell.h config.h

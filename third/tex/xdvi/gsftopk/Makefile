#
# Makefile for gsftopk.
#

### Please set the following paths.
TEXMF=/usr/local/lib/texmf

# The following three should be as in dvips.
HEADERDIR=$(TEXMF)/dvips
HEADERPATH=.:$(HEADERDIR)
CONFIGPATH=.:$(TEXMF)/dvips

# The following should be as in TeX.
TEXFONTS_DEFAULT=.:$(TEXMF)/fonts/tfm//

# Where to put things.
BINDIR=$(TEXMF)/bin
MANDIR=$(TEXMF)/man/man1

# (Optional) The fully-qualified path of Ghostscript.
# GSPATH=-DGS=\"/usr/local/gnu/bin/gs\"

# (Optional) Other definitions
# OLD_DVIPS - define if you have an old version of dvips that uses psfonts.map
#	entries of the form
#	    Symbol-Slanted "/Symbol .167 SlantFont"
#	as opposed to
#	    Symbol-Slanted Symbol ".167 SlantFont"
# DEFS=

### End of user-edited part.

# CC=gcc
# LINT=gcc -c -Wall -Wshadow -Wpointer-arith -Dlint
# also maybe -Wcast-qual -Wwrite-strings
RM=rm -f
INSTPGMMODE=755
INSTDATMODE=644
INSTMANMODE=644

CFLAGS=-O -DCONFIGPATH=\"$(CONFIGPATH)\" -DHEADERPATH=\"$(HEADERPATH)\" \
  -DTEXFONTS_DEFAULT=\"$(TEXFONTS_DEFAULT)\" $(GSPATH) $(DEFS)

default: gsftopk gsftopk.1

hpux:
	$(MAKE) CC='c89 -D_POSIX_SOURCE'

gsftopk: gsftopk.c config.h
	$(CC) -o gsftopk $(CFLAGS) gsftopk.c

config.h: Configure
	./Configure $(CC) $(CFLAGS)

gsftopk.1: gsftopk1.sed sedscript
	sed -f sedscript <gsftopk1.sed >$@

sedscript: mksedscript Makefile
	chmod u+x mksedscript
	./mksedscript $(DEFS) >$@

install: gsftopk gsftopk.1
	cp gsftopk $(BINDIR)
	chmod $(INSTPGMMODE) $(BINDIR)/gsftopk
	cp render.ps $(HEADERDIR)
	chmod $(INSTDATMODE) $(HEADERDIR)/render.ps
	cp gsftopk.1 $(MANDIR)
	chmod $(INSTMANMODE) $(MANDIR)/gsftopk.1

uninstall:
	-$(RM) $(BINDIR)/gsftopk $(HEADERDIR)/render.ps $(MANDIR)/gsftopk.1

lint:
	$(LINT) $(CFLAGS) gsftopk.c

TAGS:	$(srcs)
	etags $(srcs)

clean:
	-$(RM) *.o gsftopk gsftopk.1 sedscript core *~

mostlyclean: clean

distclean: clean
	-$(RM) config.h 

veryclean: distclean
	-$(RM) TAGS

realclean: distclean
	-$(RM) TAGS

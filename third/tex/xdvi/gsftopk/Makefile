#
# Makefile for gsftopk.
#

### Please set the following paths.

# The following three should be as in dvips.
HEADERDIR=/usr/local/tex/lib/dvips
HEADERPATH=.:$(HEADERDIR)
CONFIGPATH=.:/usr/local/tex/lib/dvips

# The following should be as in TeX.
TEXFONTS_DEFAULT=.:/usr/local/tex/lib/texmf/fonts//tfm//

# Where to put things.
BINDIR=/usr/local/tex/bin
MANDIR=/usr/man/man1

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
INSTALL=install -c
INSTPGMFLAGS=
INSTDATFLAGS=-m 644
INSTMANFLAGS=-m 644

CFLAGS=-O -DCONFIGPATH=\"$(CONFIGPATH)\" -DHEADERPATH=\"$(HEADERPATH)\" \
  -DTEXFONTS_DEFAULT=\"$(TEXFONTS_DEFAULT)\" $(GSPATH) $(DEFS)

gsftopk: gsftopk.c config.h
	$(CC) -o gsftopk $(CFLAGS) gsftopk.c

config.h: Configure
	./Configure $(CC) $(CFLAGS)

install: gsftopk
	$(INSTALL) $(INSTALLFLAGS) $(INSTPGMFLAGS) gsftopk $(BINDIR)
	$(INSTALL) $(INSTALLFLAGS) $(INSTDATFLAGS) render.ps $(HEADERDIR)
	$(INSTALL) $(INSTALLFLAGS) $(INSTMANFLAGS) gsftopk.1 $(MANDIR)

uninstall:
	$(RM) $(BINDIR)/gsftopk $(HEADERDIR)/render.ps $(MANDIR)/gsftopk.1

lint:
	$(LINT) $(CFLAGS) gsftopk.c

TAGS:	$(srcs)
	etags $(srcs)

clean:
	$(RM) *.o gsftopk core *~

mostlyclean: clean

distclean: clean
	$(RM) config.h 

veryclean: distclean
	$(RM) TAGS

realclean: distclean
	$(RM) TAGS
